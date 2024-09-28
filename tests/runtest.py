


import os
import subprocess

var_tests = [
    "./tests/variables/1.varIntDecl.cryo",
    "./tests/variables/2.varStrDecl.cryo",
    "./tests/variables/3.varBoolDecl.cryo",
    "./tests/variables/4.varExprAdd.cryo",
    "./tests/variables/5.varExprSub.cryo",
    "./tests/variables/6.varExprMult.cryo",
    "./tests/variables/7.varExprDiv.cryo",
    "./tests/variables/8.varChainExpr.cryo",
    "./tests/variables/9.varIntRef.cryo",
    "./tests/variables/10.varStrRef.cryo",
    "./tests/variables/11.varBoolRef.cryo",
    "./tests/variables/12.varExprRef.cryo",
    "./tests/variables/13.varChainExprRef.cryo"
]

func_tests = [
    "./tests/functions/functionDec/1.basicFunc.int.cryo",
    "./tests/functions/functionDec/2.varInFunc.int.cryo",
    "./tests/functions/functionDec/3.varReturnFunc.int.cryo",
    "./tests/functions/functionDec/4.strReturnFunc.str.cryo",
    "./tests/functions/functionDec/5.strVarReturnFunc.str.cryo",
    "./tests/functions/functionDec/6.boolReturnFunc.bool.cryo",
    "./tests/functions/functionDec/7.boolVarRefFunc.bool.cryo",
    "./tests/functions/functionDec/8.voidReturnFunc.void.cryo",

]

func_call_tests = [
    "./tests/functions/functionCalls/1.basicFuncCall.int.cryo",
    "./tests/functions/functionCalls/2.varFuncCall.int.cryo",
    "./tests/functions/functionCalls/3.explicitStrFuncCall.str.cryo",
    "./tests/functions/functionCalls/4.varStrFuncCall.str.cryo",
    "./tests/functions/functionCalls/5.boolFuncCall.bool.cryo",
    "./tests/functions/functionCalls/6.varBoolFuncCall.bool.cryo",
    "./tests/functions/functionCalls/7.voidFuncCall.void.cryo"
]

if_tests = [
    "./tests/ifstatement/1.basicIf.cryo"
]


cryo_path = "C:/Programming/apps/cryo/src/bin/main"
cryo_path = "/home/phock/Programming/apps/cryo/src/bin/main"

def run_test(test_file):
    if not os.path.exists(cryo_path):
        print(f"Error: Cryo compiler not found at {cryo_path}")
        return "Compiler not found"
    try:
        subprocess.run([cryo_path, test_file], capture_output=True, text=True, check=True)
        return "Passed"
    except subprocess.CalledProcessError as e:
        return f"FAILED\n\n{e.stderr.strip()}"

def run_tests(test_files):
    results = []
    for test_file in test_files:
        result = run_test(test_file)
        results.append((os.path.basename(test_file), result))
    return results

def print_results(title, results):
    print(f"\n{title}")
    print("-" * len(title))
    for test, result in results:
        status = "Passed" if "Passed" in result else "FAILED"
        print(f"Test: | {test:35}>   {status}")
        if status == "FAILED":
            error_lines = result.split('FAILED')[1].strip().split('\n')
            for line in error_lines:
                print(f"      {line}")

def summarize_results(results):
    passed = sum(1 for _, result in results if "Passed" in result)
    failed = len(results) - passed
    return passed, failed

if __name__ == "__main__":
    print("Running variable tests...")
    var_results = run_tests(var_tests)
    print_results("Variable Tests", var_results)
    
    print("\nRunning function tests...")
    func_results = run_tests(func_tests)
    print_results("Function Tests", func_results)

    print("\nRunning function call tests...")                   #
    func_call_results = run_tests(func_tests)                   # new
    print_results("Function Call Tests", func_results)          #
    
    print("\nRunning if statement tests...")
    if_results = run_tests(if_tests)
    print_results("If Statement Tests", if_results)
    
    var_passed, var_failed = summarize_results(var_results)
    func_passed, func_failed = summarize_results(func_results)
    func_call_passed, func_call_failed = summarize_results(func_call_results)   # new
    if_passed, if_failed = summarize_results(if_results)

    total_passed = var_passed + func_passed + if_passed
    total_failed = var_failed + func_failed + if_failed
    total_tests = total_passed + total_failed
    
    print("\nTest Summary")
    print("------------")
    print(f"Total tests run: {total_tests}")
    print(f"Passed: {total_passed}")
    print(f"Failed: {total_failed}")
    
    print("\nVariable Tests Summary")
    print(f"  Passed: {var_passed}")
    print(f"  Failed: {var_failed}")
    
    print("\nFunction Tests Summary")
    print(f"  Passed: {func_passed}")
    print(f"  Failed: {func_failed}")

    print("\nFunction Call Test Summary")       #
    print(f"  Passed: {func_call_passed}")      # New
    print(f"  Failed: {func_call_failed}")      #

    print("\nIf Statement Tests Summary")
    print(f"  Passed: {if_passed}")
    print(f"  Failed: {if_failed}")



    