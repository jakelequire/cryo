; ModuleID = 'foocxx.cpp'
source_filename = "foocxx.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.IntStructTesting = type { i32 }

$_ZN16IntStructTestingC2Ei = comdat any   

; This is the test function that is creating an instance of the IntStructTesting struct
; and calling the constructor with the value 34
; `void fooTest(void) { IntStructTesting i = 34; }` <-- C++ code
define dso_local void @_Z7fooTestv() #0 {   ; The function signature is mangled
  %1 = alloca %struct.IntStructTesting, align 4 ; Allocate memory for the struct
  call void @_ZN16IntStructTestingC2Ei(%struct.IntStructTesting* %1, i32 34) ; Call the constructor
  ret void
}

; This is the constructor for the IntStructTesting struct. It takes a pointer to the struct,
; and an integer value as arguments. It then assigns the integer value to the struct's value field.
; It's constructor is just a simple assignment operation inside a function.
define linkonce_odr dso_local void @_ZN16IntStructTestingC2Ei(%struct.IntStructTesting* %0, i32 %1) unnamed_addr #1 comdat align 2 {
  %3 = alloca %struct.IntStructTesting*, align 8
  %4 = alloca i32, align 4
  store %struct.IntStructTesting* %0, %struct.IntStructTesting** %3, align 8
  store i32 %1, i32* %4, align 4
  %5 = load %struct.IntStructTesting*, %struct.IntStructTesting** %3, align 8
  %6 = getelementptr inbounds %struct.IntStructTesting, %struct.IntStructTesting* %5, i32 0, i32 0
  %7 = load i32, i32* %4, align 4
  store i32 %7, i32* %6, align 4
  ret void
}

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #2 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void @_Z7fooTestv()
  ret i32 0
}

attributes #0 = { noinline optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
