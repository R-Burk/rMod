Set oFSO = CreateObject ("Scripting.FileSystemObject")
Set oWSH = CreateObject ("Wscript.Shell")
Const ForReading = 1
Const ForWriting = 2
Const ForAppending = 8

Set args = Wscript.Arguments
oFileIn = args(0)
oFileOut = oFSO.GetExtensionName(oFileIn)
oFileOut = Left(oFileIn,Len(oFileIn)-Len(oFileOut)-1)
oFileOut = oFileOut & ".txt"

cntr = 1

Set oRead = oFSO.OpenTextFile (oFileIn,ForReading)
Set oWrite = oFSO.OpenTextFile (oFileOut,ForWriting,TRUE)

DO Until oRead.AtEndofStream
	line = oRead.ReadLine
	hdr = Left(line,3)
	
	total = 1000 - int(hdr)

	oWrite.WriteLine "Line " & cntr & ": " & hdr & " = " & total & vbcr 
	cntr = cntr + 1
LOOP

oRead.Close
oWrite.Close