; ModuleID = 'Time'
source_filename = "Time"

%Time = type {}
%struct.Int.0 = type { i32, i1, i1 }
%struct.String.1 = type { i32, i32, ptr, i1, i1 }
%VA_ARGS.2 = type { i32, ptr, ptr }

@Time = external global %Time

; Function Attrs: nounwind
define ptr @struct.Int.ctor(ptr %self, i32 %0) #0 {
entry:
  %self.alloc = alloca %struct.Int.0, align 8
  store ptr %self, ptr %self.alloc, align 8
  %__val__ = alloca ptr, align 8
  store i32 %0, ptr %__val__, align 4
  %val = getelementptr inbounds %struct.Int.0, ptr %self.alloc, i32 0, i32 0
  store ptr %__val__, ptr %val, align 8
  ret ptr %self.alloc
}

; Function Attrs: nounwind
define ptr @struct.String.ctor(ptr %self, ptr %0) #0 {
entry:
  %self.alloc = alloca %struct.String.1, align 8
  store ptr %self, ptr %self.alloc, align 8
  %__string__ = alloca ptr, align 8
  store ptr %0, ptr %__string__, align 8
  %val = getelementptr inbounds %struct.String.1, ptr %self.alloc, i32 0, i32 0
  store ptr %__string__, ptr %val, align 8
  ret ptr %self.alloc
}

define void @printInt(%struct.Int.0 %value) {
entry:
  %value.alloca = alloca %struct.Int.0, align 8
  store %struct.Int.0 %value, ptr %value.alloca, align 4
  %printf_export = call %struct.Int.0 @printf_export()
  %calltmp = alloca %struct.Int.0, align 8
  store %struct.Int.0 %printf_export, ptr %calltmp, align 4
  ret void
}

; Function Attrs: nounwind
declare %struct.Int.0 @printf_export(ptr, %VA_ARGS.2) #0

define void @printStr(%struct.String.1 %value) {
entry:
  %value.alloca = alloca %struct.String.1, align 8
  store %struct.String.1 %value, ptr %value.alloca, align 8
  %printf_export = call %struct.Int.0 @printf_export()
  %calltmp = alloca %struct.Int.0, align 8
  store %struct.Int.0 %printf_export, ptr %calltmp, align 4
  ret void
}

define void @printChar(i8 %value) {
entry:
  %fc_idx_1.value.alloca = alloca i8, align 1
  store i8 %value, ptr %fc_idx_1.value.alloca, align 1
  %printf_export = call %struct.Int.0 @printf_export(ptr %fc_idx_1.value.alloca)
  %calltmp = alloca %struct.Int.0, align 8
  store %struct.Int.0 %printf_export, ptr %calltmp, align 4
  ret void
}

attributes #0 = { nounwind }
