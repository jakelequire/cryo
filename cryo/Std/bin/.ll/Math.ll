; ModuleID = 'Math'
source_filename = "Math"

%Math = type { %struct.Int.3 }
%struct.Int.3 = type { i32, i1, i1 }
%struct.String.4 = type { ptr, i32, i32, i1, i1 }
%VA_ARGS.5 = type { i32, ptr, ptr }

@Math = external global %Math

; Function Attrs: nounwind
define ptr @struct.Int.ctor(ptr %self, i32 %0) #0 {
entry:
  %self1 = alloca %struct.Int.3, align 8
  store ptr %self, ptr %self1, align 8
  %self2 = load ptr, ptr %self1, align 8
  ret ptr %self2
}

; Function Attrs: nounwind
define ptr @struct.String.ctor(ptr %self, ptr %0) #0 {
entry:
  %self1 = alloca %struct.String.4, align 8
  store ptr %self, ptr %self1, align 8
  %self2 = load ptr, ptr %self1, align 8
  ret ptr %self2
}

define void @printInt(%struct.Int.3 %value) {
entry:
  %value1 = alloca %struct.Int.3, align 8
  store %struct.Int.3 %value, ptr %value1, align 4
  %printf_export = call %struct.Int.3 @printf_export()
  %calltmp = alloca %struct.Int.3, align 8
  store %struct.Int.3 %printf_export, ptr %calltmp, align 4
  ret void
}

; Function Attrs: nounwind
declare %struct.Int.3 @printf_export(ptr, %VA_ARGS.5) #0

define void @printStr(%struct.String.4 %value) {
entry:
  %value1 = alloca %struct.String.4, align 8
  store %struct.String.4 %value, ptr %value1, align 8
  %printf_export = call %struct.Int.3 @printf_export()
  %calltmp = alloca %struct.Int.3, align 8
  store %struct.Int.3 %printf_export, ptr %calltmp, align 4
  ret void
}

define void @printChar(i8 %value) {
entry:
  %value1 = alloca i8, align 1
  store i8 %value, ptr %value1, align 1
  %value_load = load ptr, ptr %value1, align 8
  %printf_export = call %struct.Int.3 @printf_export(ptr %value_load)
  %calltmp = alloca %struct.Int.3, align 8
  store %struct.Int.3 %printf_export, ptr %calltmp, align 4
  ret void
}

attributes #0 = { nounwind }
