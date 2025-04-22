; ModuleID = 'IO'
source_filename = "IO"

%class.IO = type {}

@.str.93825121016368 = unnamed_addr constant [12 x i8] c"Placeholder\00", align 1
@IO = external global %class.IO

; Function Attrs: nounwind
declare ptr @readFile_export(ptr) #0

; Function Attrs: nounwind
define ptr @IO.readFile(ptr %filePath) #0 {
entry:
  %readFile_export = call ptr @readFile_export(ptr %filePath)
  %content = alloca ptr, align 8
  store ptr %readFile_export, ptr %content, align 8
  ret ptr @.str.93825121016368
}

attributes #0 = { nounwind }
