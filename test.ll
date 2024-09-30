; ModuleID = 'test'
source_filename = "CryoModule"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

@0 = private unnamed_addr constant [10 x i8] c"undefined\00", align 1

declare void @printInt(i32)

declare void @printStr(ptr)

declare i32 @strLength(ptr)

define void @getStringLength() {
entry:
  %testStrLength = alloca i32, align 4
  %str = alloca [8 x i8], align 1
  store [8 x i8] c"strTest\00", ptr %str, align 1
  %0 = call i32 @strLength(ptr %str)
  store i32 %0, ptr %testStrLength, align 4
  %str1 = alloca [15 x i8], align 1
  store [15 x i8] c"String length:\00", ptr %str1, align 1
  call void @printStr(ptr %str1)
  call void @printInt(i32 %0)
  ret void
}

define void @test(i32 %numberIterator) {
entry:
  %numberIterator.addr = alloca i32, i32 %numberIterator, align 4
  %pleasework = add i32 %numberIterator, 1
  %intArrTwo = alloca [10 x i32], align 4
  store [10 x i32] [i32 101, i32 102, i32 103, i32 104, i32 105, i32 106, i32 107, i32 108, i32 109, i32 110], ptr %intArrTwo, align 4
  %0 = icmp ult i32 %numberIterator, 10
  br i1 %0, label %in_bounds, label %out_of_bounds

in_bounds:                                        ; preds = %entry
  %1 = getelementptr ptr, ptr %intArrTwo, i32 %numberIterator
  %2 = load ptr, ptr %1, align 8
  br label %merge

out_of_bounds:                                    ; preds = %entry
  call void @printStr(ptr @0)
  br label %merge

merge:                                            ; preds = %in_bounds, %out_of_bounds
  %3 = phi ptr [ %2, %in_bounds ], [ undef, %out_of_bounds ]
  %4 = getelementptr i32, ptr %intArrTwo, i32 %numberIterator
  %5 = load i32, ptr %4, align 4
  %indexArrNew = alloca i32, i32 %5, align 4
  store i32 %5, ptr %indexArrNew, align 4
  %str = alloca [11 x i8], align 1
  store [11 x i8] c"----------\00", ptr %str, align 1
  call void @printStr(ptr %str)
  %str1 = alloca [14 x i8], align 1
  store [14 x i8] c"Param value: \00", ptr %str1, align 1
  call void @printStr(ptr %str1)
  call void @printInt(i32 %numberIterator)
  %str2 = alloca [14 x i8], align 1
  store [14 x i8] c"Array value: \00", ptr %str2, align 1
  call void @printStr(ptr %str2)
  %6 = load i32, ptr %indexArrNew, align 4
  call void @printInt(i32 %6)
  call void @printStr(ptr %str)
  %str3 = alloca [2 x i8], align 1
  store [2 x i8] c" \00", ptr %str3, align 1
  call void @printStr(ptr %str3)
  br label %ifCondition

ifCondition:                                      ; preds = %merge
  %ifCondition4 = icmp slt i32 %pleasework, 5
  br i1 %ifCondition4, label %thenBlock, label %elseBlock

thenBlock:                                        ; preds = %ifCondition
  call void @test(i32 %pleasework)
  br label %mergeBlock

elseBlock:                                        ; preds = %ifCondition
  ret void

mergeBlock:                                       ; preds = %thenBlock
  ret void
}

define i32 @add(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  %b.addr = alloca i32, i32 %b, align 4
  %addtmp = add i32 %a, %b
  ret i32 %addtmp
}

define i32 @subtract(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  %b.addr = alloca i32, i32 %b, align 4
  %subtmp = sub i32 %a, %b
  ret i32 %subtmp
}

define i32 @multiply(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  %b.addr = alloca i32, i32 %b, align 4
  %multmp = mul i32 %a, %b
  ret i32 %multmp
}

define i32 @divide(i32 %a, i32 %b) {
entry:
  %a.addr = alloca i32, i32 %a, align 4
  %b.addr = alloca i32, i32 %b, align 4
  %divtmp = sdiv i32 %a, %b
  ret i32 %divtmp
}

define void @main() {
entry:
  %str = alloca [14 x i8], align 1
  store [14 x i8] c"Hello, World!\00", ptr %str, align 1
  call void @printStr(ptr %str)
  %intOne = alloca i32, i32 34, align 4
  %intTwo = alloca i32, i32 35, align 4
  %intThree = alloca i32, i32 36, align 4
  %intFour = alloca i32, i32 6, align 4
  %additionResult = alloca i32, align 4
  %0 = call i32 @add(i32 34, i32 35)
  store i32 %0, ptr %additionResult, align 4
  %subtractResult = alloca i32, align 4
  %1 = call i32 @subtract(i32 35, i32 34)
  store i32 %1, ptr %subtractResult, align 4
  %multiplyResult = alloca i32, align 4
  %2 = call i32 @multiply(i32 34, i32 35)
  store i32 %2, ptr %multiplyResult, align 4
  %divideResult = alloca i32, align 4
  %3 = call i32 @divide(i32 34, i32 6)
  store i32 %3, ptr %divideResult, align 4
  call void @printInt(i32 %0)
  call void @printInt(i32 %1)
  call void @printInt(i32 %2)
  call void @printInt(i32 %3)
  %anotherAddResult = alloca i32, align 4
  %"34" = alloca i32, i32 34, align 4
  %"35" = alloca i32, i32 35, align 4
  %4 = call i32 @add(i32 34, i32 35)
  store i32 %4, ptr %anotherAddResult, align 4
  call void @printInt(i32 %4)
  %"425" = alloca i32, i32 425, align 4
  call void @printInt(i32 425)
  %str1 = alloca [20 x i8], align 1
  store [20 x i8] c"Hello, World Again!\00", ptr %str1, align 1
  call void @printStr(ptr %str1)
  %str2 = alloca [21 x i8], align 1
  store [21 x i8] c"--------------------\00", ptr %str2, align 1
  call void @printStr(ptr %str2)
  %str3 = alloca [3 x i8], align 1
  store [3 x i8] c"  \00", ptr %str3, align 1
  call void @printStr(ptr %str3)
  %intArr = alloca [5 x i32], align 4
  store [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5], ptr %intArr, align 4
  %indexArr = alloca i32, i32 4, align 4
  %intZero = alloca i32, i32 5, align 4
  %"0" = alloca i32, i32 0, align 4
  call void @test(i32 0)
  call void @printStr(ptr %str2)
  call void @getStringLength()
  ret void
}
