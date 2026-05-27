N10 G00 X100 Y100 Z100 ; Rapid move to safe start position above first corner
N20 G01 Z50 F1000 S5000 M03 ; Start spindle and feed down to base plane (Z=50)

N30 G01 X160 Y100 Z50 ; Base edge 1: (100,100,50) -> (160,100,50)
N40 G01 X160 Y140 Z50 ; Base edge 2
N50 G01 X100 Y140 Z50 ; Base edge 3
N60 G01 X100 Y100 Z50 ; Base edge 4 (close base rectangle)

N70 G01 X100 Y100 Z80 ; Vertical edge at corner A
N80 G01 X160 Y100 Z80 ; Top edge 1
N90 G01 X160 Y100 Z50 ; Vertical edge at corner B (down)
N100 G01 X160 Y100 Z80 ; Vertical edge at corner B (up, return to top)
N110 G01 X160 Y140 Z80 ; Top edge 2
N120 G01 X160 Y140 Z50 ; Vertical edge at corner C (down)
N130 G01 X160 Y140 Z80 ; Vertical edge at corner C (up, return to top)
N140 G01 X100 Y140 Z80 ; Top edge 3
N150 G01 X100 Y140 Z50 ; Vertical edge at corner D (down)
N160 G01 X100 Y140 Z80 ; Vertical edge at corner D (up, return to top)
N170 G01 X100 Y100 Z80 ; Top edge 4 (close top rectangle)
N180 G01 X100 Y100 Z50 ; Return to base at corner A

N190 G00 Z100 ; Rapid retract to safe height
