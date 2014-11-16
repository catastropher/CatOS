Imports System.Security.Cryptography
Imports Common

Public Class Main

#Region " Declarations "
  Private Const _WRONG_NUM_ARGUMENTS As Integer = 1
  Private Const _UNKNOWN_EXCEPTION As Integer = 2
  Private Const _VALIDATION_FAILED As Integer = 3

  Private Const _MAX_ARGUMENTS As Integer = 2
#End Region

#Region " Public Methods "
  Public Shared Sub Main(ByVal args() As String)
    If args.Length = 0 OrElse args.Length > _MAX_ARGUMENTS Then
      'Show usage and get out
      _DisplayUsage(_WRONG_NUM_ARGUMENTS, "Invalid number of arguments!")
      Exit Sub
    End If

    Try
      'Get all the data from the file to a byte array
      Dim file As New IO.FileStream(args(0), IO.FileMode.Open)
      Dim length As Integer = Convert.ToInt32(file.Length)
      Dim data(Convert.ToInt32(length - 1)) As Byte
      file.Read(data, 0, length)
      file.Close()

      'Calculate the MD5 hash
      Dim md5 As New MD5CryptoServiceProvider()
      Dim hash() As Byte = md5.ComputeHash(data, 0, length)

      'Sign the hash
      Dim sig As New Signature(hash)

      If args.Length > 1 Then
        'Get D and N from the key file instead
        Dim nums() As String = IO.File.ReadAllLines(args(1))
        Dim n As String = nums(0).Substring(2)
        Dim p As String = nums(1).Substring(2)
        Dim q As String = nums(2).Substring(2)

        'Reverse endianness of all strings
        n = _ReverseEndianness(n)
        p = _ReverseEndianness(p)
        q = _ReverseEndianness(q)

        sig.D = Signature.GetPrivateKeyExponent(p, q)
        sig.N = n
      End If

      If sig.Validate() Then
        'It's valid, so output the validation data (ew)
        Dim str As String = "020D"
        Dim tmp As String = sig.ToString()
        Dim ret(Convert.ToInt32(tmp.Length / 2) - 1) As Byte

        For i As Integer = ret.Length - 1 To 0 Step -1
          ret(i) = Convert.ToByte(tmp.Substring(0, 2), 16)
          tmp = tmp.Remove(0, 2)
        Next

        str &= ret.Length.ToString("X2")
        For i As Integer = 0 To ret.Length - 1
          str &= ret(i).ToString("X2")
        Next

        Console.WriteLine(str)
      Else
        Throw New SmartException(_VALIDATION_FAILED, "Generated signature did not pass validation!")
      End If
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
    Console.WriteLine("Usage: GenerateSignature <BIN file> [key file]")
    Console.WriteLine("Calculates the MD5 hash of all data in a binary file and returns the signature for it.")
    Console.WriteLine("Specify key file or leave off for default key (0005).")
    Console.WriteLine()
    Console.WriteLine("Example: GenerateSignature data.bin")
    Console.WriteLine("         GenerateSignature data.bin 0A.key")
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
#End Region

End Class
