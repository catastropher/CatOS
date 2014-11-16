Imports System.Text
Imports Common

Public Class Main

#Region " Declarations "
    Private Const _WRONG_NUM_ARGUMENTS As Integer = 1
    Private Const _UNKNOWN_EXCEPTION As Integer = 2

    Private Const _NUM_ARGUMENTS As Integer = 1
    Private Const _PAGE_SIZE As Integer = 16384
#End Region

#Region " Public Methods "
    Public Shared Sub Main(ByVal args() As String)
        If args.Length <> _NUM_ARGUMENTS Then
            'Show usage and get out
            _DisplayUsage(_WRONG_NUM_ARGUMENTS, "Invalid number of arguments!")
            Exit Sub
        End If

        Try
            Dim data(_PAGE_SIZE - 1) As Byte

            'Get the Intel Hex data
            Console.WriteLine(String.Format("Reading {0}...", args(0)))
            Dim hexFile As New IO.FileStream(args(0), IO.FileMode.Open)
            Dim header(7) As Byte
            hexFile.Read(header, 0, header.Length)
            Dim offset As Integer = 0
            If ASCIIEncoding.ASCII.GetString(header) = "**TIFL**" Then
                offset = &H4E
            End If
            Dim fileData(Convert.ToInt32(hexFile.Length - offset)) As Byte
            hexFile.Seek(offset, System.IO.SeekOrigin.Begin)
            hexFile.Read(fileData, 0, fileData.Length)
            hexFile.Close()

            Dim intelHex As New IO.StreamReader(New IO.MemoryStream(fileData), Text.ASCIIEncoding.ASCII)
            Dim address As Integer
            Dim page As Nullable(Of Integer)
            Dim pastHeader As Boolean
            While Not intelHex.EndOfStream
                Dim line As New IntelHexLine()

                Dim raw As String = intelHex.ReadLine()
                If Not String.IsNullOrWhiteSpace(raw) AndAlso Asc(raw(0)) <> 0 Then
                    line.RawStringData = raw

                    Select Case line.Type
                        Case IntelHexLine.LineType.StartEndMarker
                            If pastHeader Then
                                Exit While
                            End If

                            pastHeader = True
                        Case IntelHexLine.LineType.PageStart, IntelHexLine.LineType.OSPageStart
                            If page.HasValue Then
                                'We already had a page, so let's flush the buffer out to file
                                _FlushToFile(page.Value, data)
                            End If

                            'Get the page number
                            page = line.Data(1)

                            'Fill our buffer with 0FFh's
                            For i As Integer = 0 To data.Length - 1
                                data(i) = &HFF
                            Next
                        Case IntelHexLine.LineType.Data
                            address = line.Address And &H3FFF
                            For i As Integer = 0 To line.Data.Length - 1
                                data(address + i) = line.Data(i)
                            Next
                        Case IntelHexLine.LineType.StartSegment
                            'Do nothing
                        Case Else
                            Throw New SmartException(_UNKNOWN_EXCEPTION, _
                                    String.Format("Invalid line type {0} encountered!", line.Type))
                    End Select
                End If
            End While
            intelHex.Close()

            'Flush the last page
            If page.HasValue Then _FlushToFile(page.Value, data)

            Console.WriteLine("Extraction complete.")
        Catch ex As SmartException
            _DisplayUsage(ex.ExitCode, ex.Message)
            Exit Sub
        Catch ex As Exception
            _DisplayUsage(_UNKNOWN_EXCEPTION, ex.Message)
            Exit Sub
        End Try
    End Sub
#End Region

#Region " Local Methods "
    Private Shared Sub _DisplayUsage(ByVal exitCode As Integer, ByVal msg As String)
        Console.WriteLine(My.Application.Info.ProductName & " " & My.Application.Info.Version.ToString(3))
        Console.WriteLine("--------------------------------------------------")
        Console.WriteLine("Usage: ExtractFrom8XU <Intel Hex file>")
        Console.WriteLine("Converts 8XU/8CU Intel Hex to binary files, one per page.")
        Console.WriteLine("Page files will be named ""page[two-digit hex number].bin"".")
        Console.WriteLine()
        Console.WriteLine("Examples: ExtractFrom8XU OS.8xu")
        Console.WriteLine()
        Console.WriteLine("ERROR: " & msg)

        Environment.ExitCode = exitCode
    End Sub

    Private Shared Sub _FlushToFile(ByVal page As Integer, ByVal data() As Byte)
        Dim pageFile As New IO.FileStream(String.Format("page{0}.bin", _
            page.ToString("X02")), IO.FileMode.Create)
        pageFile.Write(data, 0, data.Length)
        pageFile.Close()
    End Sub
#End Region

End Class
