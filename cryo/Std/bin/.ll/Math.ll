; ModuleID = 'Math'
source_filename = "Math"

%class.Math = type { %struct.Int }
%struct.Int = type { i32 }

@Math = external global %class.Math
