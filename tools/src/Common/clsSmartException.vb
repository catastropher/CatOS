Public Class SmartException
    Inherits Exception

#Region " Declarations "
    Private _exitCode As Integer
    Private _message As String
#End Region

#Region " Constructors/Destructors "
    Public Sub New(ByVal exitCode As Integer, ByVal msg As String)
        _exitCode = exitCode
        _message = msg
    End Sub
#End Region

#Region " Public Properties "
    Public ReadOnly Property ExitCode() As Integer
        Get
            Return _exitCode
        End Get
    End Property
#End Region

#Region " Public Overrides "
    Public Overrides ReadOnly Property Message() As String
        Get
            Return _message
        End Get
    End Property

    Public Overrides Function ToString() As String
        Return _message
    End Function
#End Region

End Class
