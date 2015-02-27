Option Explicit

DIM oFSO, intFile, oFile, oLine, oHdr, checksum, hdr, txt, oOpen
DIM oLineFile, oOutput, oArgs
DIM fileProject, fileOutput, path, lineNum
Const Compression = 2048
Const ForReading = 1
Const ForWriting = 2
Const ForAppending = 8

SET oFSO = createobject("Scripting.FileSystemObject")
SET oArgs = Wscript.Arguments
fileProject = oArgs(0)
fileOutput  = oFSO.GetExtensionName(fileProject)
fileOutput  = Left(fileProject,Len(fileProject) - Len(fileOutput) -1) & ".efw"
path = oFSO.GetParentFolderName( fileProject)

SET oFile = oFSO.GetFile( fileProject )
IF oFSO.FileExists( fileOutput ) THEN
	oFSO.DeleteFile fileOutput
END IF
SET oOutput = oFSO.OpenTextFile ( fileOutput ,ForWriting,TRUE)
SET oOpen = oFile.OpenAsTextStream(ForReading)
hdr = "000"
lineNum = 1
Do Until oOpen.AtEndOfStream
	txt = oOpen.ReadLine
	txt = VLINK(txt)
	MakeHeader (lineNum)
	oOutput.WriteLine hdr & txt	
	lineNum = lineNum + 1
LOOP
MsgBox "Done"

FUNCTION VLINK (ByVal strLine)
	DIM strEncoded, strDecoded, strChar, intPos, x
	CONST ENCODE = "D3F4C1ENZ7P9SYM2L"
	CONST DECODE = "0123456789ABCDEF:"

	strDecoded = strLine

	FOR X = 1 to LEN(strLine)
		strChar = MID(strDecoded,x,1)
		intPos = InStr(1,DECODE,strChar,vbTextCompare)
		IF intPos = 0 OR intPos > 17 THEN
			MsgBox strLine & vbcrlf & strDecoded & vbcrlf & strChar & " " & intPos
		END IF
		strEncoded = strEncoded & MID(ENCODE,intPos,1)
	NEXT
	VLINK = strEncoded
	strEncoded = null
END FUNCTION

SUB MakeHeader (ByVal num)
	CONST fill = "000"
	
	IF LEN(num) < 3 THEN
		hdr = Left(fill,3 - LEN(num)) & num
	ELSE
		hdr = num
	END IF 
END SUB