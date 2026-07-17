N10 G00 X100 Y220 Z50 ; Move to the front-left corner of the hull
N20 G01 X280 Y220 Z50 F1000 S5000 M03 ; Front hull: top edge
N30 G01 X250 Y280 Z50 ; Front hull: right side
N40 G01 X130 Y280 Z50 ; Front hull: bottom edge
N50 G01 X100 Y220 Z50 ; Front hull: left side

N60 G01 X100 Y220 Z110 ; Connect the front and back hulls
N70 G01 X280 Y220 Z110 ; Back hull: top edge
N80 G01 X250 Y280 Z110 ; Back hull: right side
N90 G01 X130 Y280 Z110 ; Back hull: bottom edge
N100 G01 X100 Y220 Z110 ; Back hull: left side

N110 G00 X280 Y220 Z50 ; Reposition to the front-right hull corner
N120 G01 X280 Y220 Z110 ; Connect the top-right hull corners
N130 G00 X250 Y280 Z50 ; Reposition to the front lower-right corner
N140 G01 X250 Y280 Z110 ; Connect the lower-right hull corners
N150 G00 X130 Y280 Z50 ; Reposition to the front lower-left corner
N160 G01 X130 Y280 Z110 ; Connect the lower-left hull corners

N170 G00 X190 Y220 Z50 ; Move to the front base of the mast
N180 G01 X190 Y70 Z50 ; Draw the front mast
N190 G01 X190 Y70 Z110 ; Connect the mast tops
N200 G01 X190 Y220 Z110 ; Draw the back mast
N210 G01 X190 Y220 Z50 ; Connect the mast bases

N220 G00 X196 Y85 Z50 ; Move to the front sail tip
N230 G01 X265 Y190 Z50 ; Front sail: outer edge
N240 G01 X196 Y190 Z50 ; Front sail: bottom edge
N250 G01 X196 Y85 Z50 ; Close the front sail

N260 G01 X196 Y85 Z110 ; Connect the sail tips
N270 G01 X265 Y190 Z110 ; Back sail: outer edge
N280 G01 X196 Y190 Z110 ; Back sail: bottom edge
N290 G01 X196 Y85 Z110 ; Close the back sail

N300 G00 X265 Y190 Z50 ; Reposition to the front outer sail corner
N310 G01 X265 Y190 Z110 ; Connect the outer sail corners
N320 G00 X196 Y190 Z50 ; Reposition to the front lower sail corner
N330 G01 X196 Y190 Z110 ; Connect the lower sail corners

N340 G00 Z160 M05 ; Stop the spindle and retract above the model
