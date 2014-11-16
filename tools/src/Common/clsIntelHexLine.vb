Public Class IntelHexLine

#Region " Declarations "
  Private _type As LineType
  Private _address As Integer
  Private _rawStringData As String
  Private _data() As Byte

  Public Enum LineType
    Data = 0
    StartEndMarker = 1
    OSPageStart = 2
    StartSegment = 3
    PageStart = 4
  End Enum
#End Region

#Region " Public Properties "
  Public ReadOnly Property Type() As LineType
    Get
      Return _type
    End Get
  End Property

  Public Property RawStringData() As String
    Get
      Return _rawStringData
    End Get
    Set(ByVal value As String)
      _rawStringData = value

      _RefreshInfo()
    End Set
  End Property

  Public ReadOnly Property Data() As Byte()
    Get
      Return _data
    End Get
  End Property

  Public ReadOnly Property Address() As Integer
    Get
      Return _address
    End Get
  End Property
#End Region

#Region " Private Methods "
  Private Sub _RefreshInfo()
    Dim length As Integer = Convert.ToInt32(_rawStringData.Substring(1, 2), 16)

    _address = Convert.ToInt32(_rawStringData.Substring(3, 4), 16)
    _type = DirectCast(Convert.ToInt32(_rawStringData.Substring(7, 2), 16), LineType)

    Dim i As Integer = 9
    _data = New Byte(length - 1) {}
    For j As Integer = 0 To length - 1
      _data(j) = Convert.ToByte(_rawStringData.Substring(i, 2), 16)
      i += 2
    Next
  End Sub
#End Region

End Class
