Imports System.Security.Cryptography
Imports Common

Public Class Main

#Region " Declarations "
  Private Const _UNKNOWN_EXCEPTION As Integer = 1
  Private Const _INVALID_PARAMETERS As Integer = 2

  Private Const _BUFFER_SIZE As Integer = (&HFF * &H4000) + 128
  Private Const _LINES_PER_PAGE As Integer = 512
  Private Const _BYTES_PER_LINE As Integer = 32

  Private Shared _data(_BUFFER_SIZE - 1) As Byte
  Private Shared _i As Integer
#End Region

#Region " Public Methods "
  Public Shared Sub Main(ByVal args() As String)
    Try
      'Parse all the command line arguments
      Dim pages As New Dictionary(Of Integer, KeyValuePair(Of Integer, String))
      Dim key As Integer
      Dim majorVersion As Integer
      Dim minorVersion As Integer
      Dim maxHardwareVersion As Integer
      Dim keyFile As String = String.Empty
      Dim outputFileName As String = String.Empty
      Dim is73 As Boolean
      Dim isColor As Boolean
      Dim validationFile As String = String.Empty

      For Each arg As String In args
        Dim switch As String = arg.Substring(0, 2).ToLower().Trim(New Char() {"-"c, "/"c})
        Dim data As String = arg.Substring(2, arg.Length - 2)

        Select Case switch
          Case "v"
            validationFile = arg.Substring(2)
          Case "c"
            isColor = True
          Case "t"
            is73 = (data = "73")
          Case "f"
            keyFile = data
          Case "k"
            key = Convert.ToInt32(data, 16)
          Case "m"
            majorVersion = Convert.ToInt32(data, 16)
          Case "n"
            minorVersion = Convert.ToInt32(data, 16)
          Case "h"
            maxHardwareVersion = Convert.ToInt32(data, 16)
          Case "o"
            outputFileName = data
          Case Else
            'This must be OS data
            Dim pieces() As String = arg.Split(":"c)

            If pieces.Length <> 3 Then
              Throw New SmartException(_INVALID_PARAMETERS, "Invalid page criteria!")
            End If

            Dim page As Integer
            If Not isColor Then
              page = Convert.ToInt32(pieces(0), 16) And &H1F
            Else
              page = Convert.ToInt32(pieces(0), 16)
            End If
            Dim offset As Integer = Convert.ToInt32(pieces(1), 16)
            pages.Add(page, New KeyValuePair(Of Integer, String)(offset, pieces(2)))
        End Select
      Next

      Console.WriteLine("Building OS data...")

      'Fill our buffer with 0FFh's
      For i As Integer = 0 To _data.Length - 1
        _data(i) = &HFF
      Next

      'Build the OS header
      _AppendData(New Byte() {&H80, &HF, &H0, &H0, &H0, &H0})
      _AppendData(New Byte() {&H80, &H11, Convert.ToByte(key)})
      _AppendData(New Byte() {&H80, &H21, Convert.ToByte(majorVersion)})
      _AppendData(New Byte() {&H80, &H31, Convert.ToByte(minorVersion)})
      _AppendData(New Byte() {&H80, &HA1, Convert.ToByte(maxHardwareVersion)})
      _AppendData(New Byte() {&H80, &H81, Convert.ToByte(pages.Count)})
      _AppendData(New Byte() {&H80, &H7F, &H0, &H0, &H0, &H0})

      'Pull the data for each page and append it to our buffer
      For Each page As KeyValuePair(Of Integer, KeyValuePair(Of Integer, String)) In pages
        Dim pg As Integer = page.Key
        Dim offset As Integer = page.Value.Key
        Dim fileName As String = page.Value.Value

        Dim file As New IO.FileStream(fileName, IO.FileMode.Open)
        file.Seek(offset, IO.SeekOrigin.Begin)
        file.Read(_data, _i, &H4000)
        _i += &H4000
        file.Close()
      Next

      Console.WriteLine("Hashing OS data...")

      'Calculate the MD5 hash
      Dim md5 As New MD5CryptoServiceProvider()
      Dim hash() As Byte = md5.ComputeHash(_data, 0, _i)

      'Sign the hash
      Dim sig As New Signature(hash)

      If Not String.IsNullOrEmpty(keyFile) Then
        Console.WriteLine(String.Format("Key file {0} specified, reading contents...", keyFile))

        'Get D and N from the key file instead
        Dim nums() As String = IO.File.ReadAllLines(keyFile)
        Dim n As String = nums(0).Substring(2)
        Dim p As String = nums(1).Substring(2)
        Dim q As String = nums(2).Substring(2)

        'Reverse endianness of all strings
        n = _ReverseEndianness(n)
        p = _ReverseEndianness(p)
        q = _ReverseEndianness(q)

        sig.D = Signature.GetPrivateKeyExponent(p, q)
        sig.N = n
      Else
        Console.WriteLine("No key file specified, defaulting to 0005...")
      End If

      'We should now have everything we need to write the 8XU file
      '(header, OS page data, signature)
      Console.WriteLine("Building 8XU...")

      'Write 73U/8XU file header
      Dim upgradeFile As New IO.FileStream(outputFileName, IO.FileMode.Create)
      Dim deviceType As Byte = &H73
      If is73 Then deviceType = &H74
      Dim headerData() As Byte = {&H2A, &H2A, &H54, &H49, &H46, &H4C, &H2A, &H2A, Convert.ToByte(majorVersion), Convert.ToByte(minorVersion), _
                                  &H1, &H88, &H11, &H26, &H20, &H7, &H8, &H62, &H61, &H73, &H65, _
                                  &H63, &H6F, &H64, &H65, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, _
                                  &H0, &H0, &H0, deviceType, &H23, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, _
                                  &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0, &H0}
      upgradeFile.Write(headerData, 0, headerData.Length)

      'Write OS header
      Dim str As String = _GetIntelHexString(0, _data, 0, &H1B)
      upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(str), 0, str.Length)
      str = ":00000001FF" & Environment.NewLine
      upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(str), 0, str.Length)

      'Write each page out
      Dim pageHeaderBase As String = ":0200000200"
      Dim pageHeader As String
      Dim checksum As Integer
      Dim ptr As Integer
      For Each pageData As KeyValuePair(Of Integer, KeyValuePair(Of Integer, String)) In pages
        pageHeader = pageHeaderBase & pageData.Key.ToString("X2") & (&HFC - pageData.Key).ToString("X2") & Environment.NewLine
        upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(pageHeader), 0, pageHeader.Length)

        For i As Integer = 0 To _LINES_PER_PAGE - 1
          Dim line As String
          Dim address As Integer = i * _BYTES_PER_LINE
          If pageData.Key <> 0 Then address = address Or &H4000

          checksum = _BYTES_PER_LINE
          line = ":20" & address.ToString("X4") & "00"
          checksum += address And &HFF
          checksum += ((address And &HFF00) >> 8) And &HFF

          For j As Integer = 0 To _BYTES_PER_LINE - 1
            Dim data As Byte = _data((address And &H3FFF) + j + (ptr * &H4000) + &H1B)
            line &= data.ToString("X2")
            checksum += data
          Next

          line &= (((Not (checksum And &HFF) And &HFF) + 1) And &HFF).ToString("X2") & Environment.NewLine

          upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(line), 0, line.Length)
        Next

        ptr += 1
      Next

      Console.WriteLine("Writing validation data...")

      'Write validation data
      str = ":00000001FF" & Environment.NewLine
      upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(str), 0, str.Length)
      Dim validationData(&H43 - 1) As Byte
      Dim tmp As String = sig.ToString()
      Dim ret(Convert.ToInt32(tmp.Length / 2) - 1) As Byte
      For i As Integer = ret.Length - 1 To 0 Step -1
        ret(i) = Convert.ToByte(tmp.Substring(0, 2), 16)
        tmp = tmp.Remove(0, 2)
      Next
      If Not String.IsNullOrEmpty(validationFile) Then
        ret = System.IO.File.ReadAllBytes(validationFile)
      End If
      'ret now contains the validation data as we should output it
      validationData(0) = &H2
      validationData(1) = &HD
      validationData(2) = &H40
      For i As Integer = 0 To ret.Length - 1
        validationData(i + 3) = ret(i)
      Next
      str = _GetIntelHexString(0, validationData, 0, &H20)
      upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(str), 0, str.Length)
      str = _GetIntelHexString(&H20, validationData, &H20, &H20)
      upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(str), 0, str.Length)
      str = _GetIntelHexString(&H40, validationData, &H40, &H3)
      upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(str), 0, str.Length)
      str = ":00000001FF   -- CONVERT 2.6 --" & Environment.NewLine
      upgradeFile.Write(System.Text.ASCIIEncoding.ASCII.GetBytes(str), 0, str.Length)

      'Close 8XU file
      upgradeFile.Close()

      'Re-open 8XU file and write correct data length and checksum
      Dim info As New IO.FileInfo(outputFileName)
      upgradeFile = New IO.FileStream(outputFileName, IO.FileMode.Open)
      upgradeFile.Seek(&H4A, IO.SeekOrigin.Begin)
      upgradeFile.Write(BitConverter.GetBytes(info.Length - &H4E), 0, 4)
      'For i As Integer = 0 To Convert.ToInt32(info.Length) - &H4E
      ' checksum += upgradeFile.ReadByte()
      'Next
      'upgradeFile.Write(BitConverter.GetBytes(Convert.ToUInt16(checksum And &HFFFF)), 0, 2)
      upgradeFile.Close()

      Console.WriteLine("Done.")
    Catch ex As SmartException
      _DisplayUsage(ex.ExitCode, ex.Message)
      Exit Sub
    Catch ex As Exception
      _DisplayUsage(_UNKNOWN_EXCEPTION, ex.ToString())
      Exit Sub
    End Try
  End Sub
#End Region

#Region " Local Methods "
  Private Shared Sub _DisplayUsage(ByVal exitCode As Integer, ByVal msg As String)
    Console.WriteLine(My.Application.Info.ProductName & " " & My.Application.Info.Version.ToString(3))
    Console.WriteLine("--------------------------------------------------")
    Console.WriteLine("Usage: Build8XU -t[83P|73] -k[OS key] -m[major ver] -n[minor ver] -h[max. HW ver]")
    Console.WriteLine("                -o[output name] -f[key file name] <[page]:[offset]:[filename]> ...")
    Console.WriteLine("Builds a valid 8XU OS upgrade file from one or more binary files.")
    Console.WriteLine()
    Console.WriteLine("Examples: to build a 2-page OS from a 32KB ROM:")
    Console.WriteLine("            Build8XU -k05 -m00 -n01 -h03 -oOS.8xu 00:0000:os.rom 01:4000:os.rom")
    Console.WriteLine("          to build a 3-page OS from separate binary files:")
    Console.WriteLine("            Build8XU -k05 -m02 -n2B -h03 -oOS.8xu 00:0000:page0.bin 01:0000:page1.bin 7C:0000:page2.bin")
    Console.WriteLine("          to build a 2-page OS from BIN files and a ROM image:")
    Console.WriteLine("            Build8XU -k05 -m02 -n42 -h03 -oOS.8xu 00:0000:page0.bin 01:4000:83p.rom 7C:7C0000:83p.rom")
    Console.WriteLine("          to build a 1-page OS from a binary file, signed with 0A.key:")
    Console.WriteLine("            Build8XU -k0A -m02 -n42 -h03 -oOS.8xu -f0A.key 00:0000:data.bin")
    Console.WriteLine("All parameters are required. All values are hexadecimal.")
    Console.WriteLine()
    Console.WriteLine("New optional parameter: -c, to specify 84+CSE.")
    Console.WriteLine()
    Console.WriteLine("All files must reside in the current directory (because I'm a lazy coder).")
    Console.WriteLine()
    Console.WriteLine("ERROR: " & msg)

    Environment.ExitCode = exitCode
  End Sub

  Private Shared Function _ReverseEndianness(ByVal s As String) As String
    Dim ret As String = String.Empty

    For i As Integer = 0 To Convert.ToInt32(s.Length / 2) - 1
      ret = s.Substring(0, 2) & ret
      s = s.Remove(0, 2)
    Next

    Return ret
  End Function

  Private Shared Sub _AppendData(ByVal b As Byte)
    _data(_i) = b
    _i += 1
  End Sub

  Private Shared Sub _AppendData(ByVal a() As Byte)
    For Each b As Byte In a
      _AppendData(b)
    Next
  End Sub

  Private Shared Function _GetIntelHexString(ByVal address As Integer, _
        ByVal data() As Byte, ByVal offset As Integer, ByVal count As Integer) As String
    Dim ret As String = ":"

    ret &= count.ToString("X2")
    ret &= address.ToString("X4")
    ret &= "00"
    For i As Integer = offset To (count + offset) - 1
      ret &= data(i).ToString("X2")
    Next
    ret &= CalculateChecksum(ret).ToString("X2")
    ret &= Environment.NewLine

    Return ret
  End Function

  Friend Shared Function CalculateChecksum(ByVal line As String) As Short
    Return CalculateChecksum(StringToShortArray(line))
  End Function

  Friend Shared Function StringToShortArray(ByVal line As String) As Short()
    Dim ret As Short() = Nothing
    line = line.Trim().TrimStart(":"c).Trim()

    If line.Length Mod 2 <> 0 Then
      Throw New ArgumentException("Invalid checksum line!")
    Else
      Dim length As Integer = Convert.ToInt32(line.Length / 2)

      Array.Resize(ret, length)

      For i As Integer = 0 To length - 1
        ret(i) = Convert.ToInt16(line.Substring(0, 2), 16)
        line = line.Remove(0, 2)
      Next
    End If

    Return ret
  End Function

  Friend Shared Function CalculateChecksum(ByVal line() As Short) As Short
    Dim ret As Short

    For Each s As Short In line
      ret = Convert.ToInt16((ret + s) And 255)
    Next

    ret = Convert.ToInt16((Not ret) And 255)
    ret += Convert.ToInt16(1)

    Return Convert.ToInt16(ret And 255)
  End Function
#End Region

End Class
