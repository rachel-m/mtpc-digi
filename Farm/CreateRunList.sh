#!/bin/tcsh

set RunListFile=RunList.txt

@ i = 1
while ($i <= 500)
@ no = $i * 10000
echo "$no" >>  $RunListFile
@ i = $i + 1
end


