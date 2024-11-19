; ModuleID = 'foo.cpp'
source_filename = "foo.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.Int = type { i32 }
%class.FooClass = type { i8 }

$_ZN3IntC2Ei = comdat any

$_ZN3Int8toStringEv = comdat any

$_ZN8FooClassC2Ev = comdat any

@.str = private unnamed_addr constant [6 x i8] c"Hello\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i8* @atoi(i32 %0) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  ret i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0)
}

; Function Attrs: noinline norecurse optnone uwtable
define dso_local i32 @main() #1 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.Int, align 4
  %3 = alloca i32, align 4
  %4 = alloca %class.FooClass, align 1
  store i32 0, i32* %1, align 4
  call void @_ZN3IntC2Ei(%struct.Int* %2, i32 5)
  %5 = getelementptr inbounds %struct.Int, %struct.Int* %2, i32 0, i32 0
  %6 = load i32, i32* %5, align 4
  store i32 %6, i32* %3, align 4
  call void @_ZN3Int8toStringEv(%struct.Int* %2)
  call void @_ZN8FooClassC2Ev(%class.FooClass* %4)
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN3IntC2Ei(%struct.Int* %0, i32 %1) unnamed_addr #0 comdat align 2 {
  %3 = alloca %struct.Int*, align 8
  %4 = alloca i32, align 4
  store %struct.Int* %0, %struct.Int** %3, align 8
  store i32 %1, i32* %4, align 4
  %5 = load %struct.Int*, %struct.Int** %3, align 8
  %6 = load i32, i32* %4, align 4
  %7 = getelementptr inbounds %struct.Int, %struct.Int* %5, i32 0, i32 0
  store i32 %6, i32* %7, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN3Int8toStringEv(%struct.Int* %0) #0 comdat align 2 {
  %2 = alloca %struct.Int*, align 8
  %3 = alloca i8*, align 8
  store %struct.Int* %0, %struct.Int** %2, align 8
  %4 = load %struct.Int*, %struct.Int** %2, align 8
  %5 = getelementptr inbounds %struct.Int, %struct.Int* %4, i32 0, i32 0
  %6 = load i32, i32* %5, align 4
  %7 = call i8* @atoi(i32 %6)
  store i8* %7, i8** %3, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define linkonce_odr dso_local void @_ZN8FooClassC2Ev(%class.FooClass* %0) unnamed_addr #0 comdat align 2 {
  %2 = alloca %class.FooClass*, align 8
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store %class.FooClass* %0, %class.FooClass** %2, align 8
  %6 = load %class.FooClass*, %class.FooClass** %2, align 8
  store i32 5, i32* %3, align 4
  store i32 10, i32* %4, align 4
  %7 = load i32, i32* %3, align 4
  %8 = load i32, i32* %4, align 4
  %9 = add nsw i32 %7, %8
  store i32 %9, i32* %5, align 4
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0-4ubuntu1 "}
