Imports Common

Public Class Main

#Region " Declarations "
  Private Const _WRONG_NUM_ARGUMENTS As Integer = 1
  Private Const _UNKNOWN_EXCEPTION As Integer = 2
  Private Const _INVALID_MODEL As Integer = 3

  Private Const _NUM_ARGUMENTS As Integer = 3
  Private Const _PAGE_SIZE As Integer = 16384
#End Region

#Region " Public Methods "
  Public Shared Sub Main(ByVal args() As String)
    If args.Length <> _NUM_ARGUMENTS Then
      'Show usage and get out
      _DisplayUsage(_WRONG_NUM_ARGUMENTS, "Invalid number of arguments!")
      Exit Sub
    ElseIf Not _IsValidModel(args(0)) Then
      'Show usage and get out
      _DisplayUsage(_INVALID_MODEL, "Invalid calculator model specified!")
      Exit Sub
    End If

    Try
      'Use the model parameter to determine size of buffer
      Dim size As Integer
      Select Case args(0).ToUpper()
        Case "83P"
          size = _PAGE_SIZE * &H20
        Case "84P"
          size = _PAGE_SIZE * &H40
        Case "SE"
          size = _PAGE_SIZE * &H80
        Case "84PCSE"
          size = _PAGE_SIZE * &H100
        Case Else
          Throw New SmartException(_INVALID_MODEL, "Invalid calculator model specified!")
      End Select
      Dim data(size - 1) As Byte

      'Fill our buffer with 0FFh's
      For i As Integer = 0 To data.Length - 1
        data(i) = &HFF
      Next

      'Read in one line at a time and store data to appropriate location in buffer
      Console.WriteLine(String.Format("Reading {0}...", args(1)))
      Dim hexFile As New IO.StreamReader(args(1), System.Text.Encoding.ASCII)
      Dim address As Integer
      Dim page As Integer
      While Not hexFile.EndOfStream
        Dim line As New IntelHexLine()

        line.RawStringData = hexFile.ReadLine()

        Select Case line.Type
          Case IntelHexLine.LineType.StartEndMarker
            Exit While
          Case IntelHexLine.LineType.PageStart
            page = line.Data(1)

            If page >= &H10 Then
              Select Case args(0).ToUpper()
                Case "83P"
                  page = page And &H1F
                Case "84P"
                  page = page Or &H30
                Case "SE"
                  page = page Or &H60
              End Select
            End If
          Case IntelHexLine.LineType.Data
            address = page * _PAGE_SIZE
            address += line.Address And &H3FFF
            For i As Integer = 0 To line.Data.Length - 1
              data(address + i) = line.Data(i)
            Next
          Case IntelHexLine.LineType.StartSegment
            'Do nothing
          Case Else
            Throw New SmartException(_UNKNOWN_EXCEPTION, _
                    String.Format("Invalid line type {0} encountered!", line.Type))
        End Select
      End While
      hexFile.Close()

      'Write the buffer out to a file
      Console.WriteLine(String.Format("Writing {0}...", args(2)))
      Dim outputFile As New IO.FileStream(args(2), IO.FileMode.Create, IO.FileAccess.Write)
      For i As Integer = 0 To size - 1
        outputFile.WriteByte(data(i))
      Next
      outputFile.Close()

      Console.WriteLine("Conversion complete.")
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
    Console.WriteLine("Usage: Hex2ROM <83P|84P|SE|84PCSE> <Intel Hex file> <ROM file>")
    Console.WriteLine("Converts Intel Hex ZDS output file to calculator ROM file.")
    Console.WriteLine()
    Console.WriteLine("Examples: Hex2ROM 83P data.hex image.rom")
    Console.WriteLine()
    Console.WriteLine("ERROR: " & msg)

    Environment.ExitCode = exitCode
  End Sub

  Private Shared Function _IsValidModel(ByVal model As String) As Boolean
    Dim ret As Boolean

    Select Case model.ToUpper()
      Case "83P", "84P", "SE", "84PCSE"
        ret = True
      Case Else
        ret = False
    End Select

    Return ret
  End Function
#End Region

End Class
