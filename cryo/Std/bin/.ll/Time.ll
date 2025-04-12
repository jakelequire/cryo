; ModuleID = 'Time'
source_filename = "Time"

%Time = type {}
%struct.Int = type { i32, i1, i1 }
%struct.String = type { ptr, i32, i32, i1, i1 }
%VA_ARGS = type { i32, ptr, ptr }

@Time = external global %Time

; Function Attrs: nounwind
define ptr @struct.Int.ctor(ptr %self, i32 %0) #0 {
entry:
  %self1 = alloca %struct.Int, align 8
  store ptr %self, ptr %self1, align 8
  %self2 = load ptr, ptr %self1, align 8
  ret ptr %self2
}

; Function Attrs: nounwind
define ptr @struct.String.ctor(ptr %self, ptr %0) #0 {
entry:
  %self1 = alloca %struct.String, align 8
  store ptr %self, ptr %self1, align 8
  %self2 = load ptr, ptr %self1, align 8
  ret ptr %self2
}

define void @printInt(%struct.Int %value) {
entry:
  %value1 = alloca %struct.Int, align 8
  store %struct.Int %value, ptr %value1, align 4
  %printf_export = call %struct.Int @printf_export()
  %calltmp = alloca %struct.Int, align 8
  store %struct.Int %printf_export, ptr %calltmp, align 4
  ret void
}

; Function Attrs: nounwind
declare %struct.Int @printf_export(ptr, %VA_ARGS) #0

define void @printStr(%struct.String %value) {
entry:
  %value1 = alloca %struct.String, align 8
  store %struct.String %value, ptr %value1, align 8
  %printf_export = call %struct.Int @printf_export()
  %calltmp = alloca %struct.Int, align 8
  store %struct.Int %printf_export, ptr %calltmp, align 4
  ret void
}

define void @printChar(i8 %value) {
entry:
  %value1 = alloca i8, align 1
  store i8 %value, ptr %value1, align 1
  %value_load = load ptr, ptr %value1, align 8
  %printf_export = call %struct.Int @printf_export(ptr %value_load)
  %calltmp = alloca %struct.Int, align 8
  store %struct.Int %printf_export, ptr %calltmp, align 4
  ret void
}

attributes #0 = { nounwind }
