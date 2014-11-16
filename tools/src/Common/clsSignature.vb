Public Class Signature

#Region " Declarations "
  Private Const _RADIX As Integer = 16
  Private Const _DEFAULT_D As String = "70B9C23D9EF0E072259990AF5538C5A0F3CE57F" & _
      "379F2059B8149915A27A9C7050D1889078AC306D98A0154CFDDD44F74B7AB2DFA44643FEBF0E0916063D631E1"
  Private Const _DEFAULT_N As String = "BFA2309BF4997D8ED9850F907746E9919E78625" & _
      "11C1B6FEEC23043E6103A38BD84F5421AD04980F79D4EC7D6093D1D1FEF60334E93BF6CD46F82F19B7EF2AB6B"
  Private _d As New BigInteger(_DEFAULT_D, _RADIX)
  Private _n As New BigInteger(_DEFAULT_N, _RADIX)
  Private _hash As BigInteger
#End Region

#Region " Constructors/Teardown "
  Public Sub New(ByVal md5Hash() As Byte)
    'Convert this to a string
    _hash = New BigInteger(_GetString(md5Hash), _RADIX)
  End Sub

  Public Sub New(ByVal md5Hash As String)
    _hash = New BigInteger(md5Hash, _RADIX)
  End Sub
#End Region

#Region " Public Properties "
  Public Property D() As String
    Get
      Return _GetBigIntegerString(_d.getBytes(), _d.getBytes().Length - 1)
    End Get
    Set(ByVal value As String)
      _d = New BigInteger(value, 16)
    End Set
  End Property

  Public Property N() As String
    Get
      Return _GetBigIntegerString(_n.getBytes(), _n.getBytes().Length - 1)
    End Get
    Set(ByVal value As String)
      _n = New BigInteger(value, 16)
    End Set
  End Property
#End Region

#Region " Public Overrides "
  Public Overrides Function ToString() As String
    Dim sig() As Byte = _hash.modPow(_d, _n).getBytes()

    Return _GetBigIntegerString(sig, sig.Length - 1)
  End Function
#End Region

#Region " Public Methods "
  Public Function Validate() As Boolean
    Dim calculatedHash As New BigInteger(Me.ToString(), _RADIX)
    Dim ret As Boolean = True

    'Calculate the hash
    calculatedHash = calculatedHash.modPow(New BigInteger(17), _n)

    'Compare our real hash and the calculated hash
    Dim ourHash() As Byte = _hash.getBytes()
    Dim myHash() As Byte = calculatedHash.getBytes()
    If ourHash.Length = myHash.Length Then
      For i As Integer = 0 To ourHash.Length - 1
        If ourHash(i) <> myHash(i) Then
          ret = False

          Exit For
        End If
      Next
    Else
      'Not even the right length, it's crap
      ret = False
    End If

    Return ret
  End Function

  Public Shared Function GetPrivateKeyExponent(ByVal p As String, ByVal q As String) As String
    Dim e As New BigInteger(CLng(17))
    Dim d As BigInteger = e.modInverse((New BigInteger(p, _RADIX) - 1) * (New BigInteger(q, _RADIX) - 1))

    Return d.ToString(_RADIX)
  End Function
#End Region

#Region " Private Methods "
  Private Function _GetString(ByVal data() As Byte) As String
    Dim ret As String = String.Empty

    For i As Integer = data.Length - 1 To 0 Step -1
      ret = ret & data(i).ToString("X2")
    Next

    Return ret
  End Function

  Private Function _GetBigIntegerString(ByVal data() As Byte, ByVal length As Integer) As String
    Dim ret As String = String.Empty

    For i As Integer = data.Length - 1 To 0 Step -1
      ret = data(i).ToString("X2") & ret
    Next

    Return ret
  End Function
#End Region

End Class
