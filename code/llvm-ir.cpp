#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/TargetSelect.h"

#include <cstdint>
#include <memory>
#include <vector>

int main(int argc, char *argv[])
{
  using namespace llvm;

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();

  LLVMContext C;
  auto builder = std::make_unique<IRBuilder<>>(C);
  auto M = std::make_unique<Module>("main", C);

  auto i32 = builder->getInt32Ty();
  auto funcTy = FunctionType::get(i32, {i32, i32}, /*isVarArg=*/false);
  auto func = Function::Create(funcTy, GlobalValue::LinkageTypes::ExternalLinkage, "func", *M);
  auto BB = BasicBlock::Create(C, "entry", func);
  builder->SetInsertPoint(BB);
  auto addVal = builder->CreateAdd(func->getArg(0), func->getArg(1));
  builder->CreateRet(addVal);
  M->print(errs(), /*AAW=*/nullptr);

  auto EE = EngineBuilder(std::move(M)).setEngineKind(llvm::EngineKind::JIT).create();

  using FuncPtr_t = uint32_t (*)(uint32_t, uint32_t);
  auto pFunc = (FuncPtr_t)EE->getFunctionAddress("func");
  auto ret = pFunc(42, 7);
  errs() << "\nfunc() returned " << ret << "\n";

  return 0;
}
