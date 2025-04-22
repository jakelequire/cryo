; ModuleID = 'Time'
source_filename = "Time"

%class.Time = type {}

@.str.93825036504608 = unnamed_addr constant [25 x i8] c"Current Time Placeholder\00", align 1
@.str.93825105008240 = unnamed_addr constant [27 x i8] c"Formatted Date Placeholder\00", align 1
@Time = external global %class.Time

; Function Attrs: nounwind
define ptr @Time.getCurrentTime() #0 {
entry:
  ret ptr @.str.93825036504608
}

; Function Attrs: nounwind
define ptr @Time.formatDate(ptr %date) #0 {
entry:
  ret ptr @.str.93825105008240
}

attributes #0 = { nounwind }
