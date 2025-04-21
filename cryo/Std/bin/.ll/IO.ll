; ModuleID = 'IO'
source_filename = "IO"

%class.IO = type {}

@IO = external global %class.IO

; Function Attrs: nounwind
declare ptr @readFile_export(ptr) #0

attributes #0 = { nounwind }
