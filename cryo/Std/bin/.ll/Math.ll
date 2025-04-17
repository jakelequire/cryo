; ModuleID = 'Math'
source_filename = "Math"

%Math = type { %struct.Int }
%struct.Int = type { i32, i1, i1 }

@Math = external global %Math
