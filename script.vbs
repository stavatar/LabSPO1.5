Const HKEY_LOCAL_MACHINE = &H80000002

Const REG_SZ = 1
Const REG_EXPAND_SZ = 2
Const REG_BINARY = 3
Const REG_DWORD = 4
Const REG_MULTI_SZ = 7
Const REG_QWORD = 11

strComputer = "."

Set objFSO=CreateObject("Scripting.FileSystemObject")

Set oReg=GetObject("winmgmts:{impersonationLevel=impersonate}!\\" & _
    strComputer & "\root\default:StdRegProv")

result = ""

dotPath = ""
slashPath = "SYSTEM\Keyboard Layout"

outFile = "commands.txt"
Set objFile = objFSO.CreateTextFile(outFile, True)

Sub getEnumKeys(slashPath, dotPath, n)
    rc = oReg.EnumKey(HKEY_LOCAL_MACHINE, slashPath, arrSubKeys)
    If rc = 0 and Not IsNull(arrSubKeys) Then
        For Each subkey In arrSubKeys

            If n = 0 Then
                result = ""
                result = result & "create " & subkey
                objFile.Write result & vbCrlf

                getValues slashPath & "\" & subkey, subkey

                getEnumKeys slashPath & "\" & subkey, subkey, n+1
            Else
                result = ""
                result = result & "create " & dotPath & "." & subkey
                objFile.Write result & vbCrlf

                getValues slashPath & "\" & subkey, dotPath & "." & subkey

                getEnumKeys slashPath & "\" & subkey, dotPath & "." & subkey, n+1
            End If
        Next
    End if
End Sub

Sub getValues(slashPath, dotPath)
    oReg.EnumValues HKEY_LOCAL_MACHINE, slashPath, arrValueNames, arrValueTypes

    If Not IsNull(arrValueNames) Then
        For i = 0 To UBound(arrValueNames)
            result = ""

            If arrValueNames(i) = "" Then
                result = result & "create " & dotPath & ".default" & "["
            Else
                result = result & "create " & dotPath & "." & arrValueNames(i) & "["
            End If

            Select Case arrValueTypes(i)
                Case REG_SZ
                    oReg.GetStringValue HKEY_LOCAL_MACHINE, slashPath, arrValueNames(i), strValue
                    result = result & strValue
                Case REG_EXPAND_SZ
                    oReg.GetExpandedStringValue HKEY_LOCAL_MACHINE, slashPath, arrValueNames(i), strValue
                    result = result & strValue
                Case REG_BINARY
                    result = result & "REG_BINARY"
                Case REG_DWORD
                    oReg.GetDWORDValue HKEY_LOCAL_MACHINE, slashPath, arrValueNames(i), dwValue
                    result = result & dwValue
                Case REG_MULTI_SZ
                    oReg.GetMultiStringValue HKEY_LOCAL_MACHINE, slashPath, arrValueNames(i), arrValues
                    multiString = ""
                    For Each strValue In arrValues
                        multiString = multiString & " " & strValue
                    Next
                    result = result & multiString
                Case REG_QWORD
                    oReg.GetQWORDValue HKEY_LOCAL_MACHINE, slashPath, arrValueNames(i), uValue
                    result = result & uValue
            End Select

            result = result & "]"
            objFile.Write result & vbCrlf
        Next
    End If

End Sub

getEnumKeys slashPath, dotPath, 0

objFile.Close