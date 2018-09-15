del %temp%\EditorLineEnds*.tt 
set dt=%date%_%time:~3,2%-%time:~6,2%
rename "%temp%\EditorLineEnds.ttr" "EditorLineEnds_%dt%.ttr"
start "" "C:\Program Files\Borland\BDS\4.0\Bin\bds.exe"