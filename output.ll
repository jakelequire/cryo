; ModuleID = 'CryoModule'
source_filename = "CryoModule"

%String = type { ptr, i32, i32, i32 }

@0 = private constant [14 x i8] c"Hello, world!\00"
@exampleStr = external global %String
@exampleStr.1 = global %String { ptr @0, i32 13, i32 13, i32 16 }
