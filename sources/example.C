#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"

#include <string>

// Example 1: create an instance of class BPatch
BPatch bpatch;

// Example 2: attaching, creating, or opening a file for rewrite
typedef enum {
  create,
  attach,
  open } accessType_t;
BPatch_addressSpace *startInstrumenting(accessType_t accessType,
					const char *name,
					int pid, // For attach
					const char *argv[]) { // For create
  BPatch_addressSpace *handle = NULL;
  switch (accessType) {
  case create:
    handle = bpatch.processCreate(name, argv);
    break;
  case attach:
    handle = bpatch.processAttach(name, pid);
    break;
  case open:
    handle = bpatch.openBinary(name);
    break;
  }
  return handle;
}

// Example 2: find the entry point for "InterestingProcedure"
std::vector<BPatch_point *> *findEntryPoint(BPatch_addressSpace *app) {
  std::vector<BPatch_function *> functions;
  std::vector<BPatch_point *> *points;

  BPatch_image *appImage = app->getImage();
  appImage->findFunction("InterestingProcedure", functions);
  points = functions[0]->findPoint(BPatch_entry);
  return points;
}

// Example 3: create and insert an increment snippet
void createAndInsertSnippet(BPatch_addressSpace *app,
			    std::vector<BPatch_point *> *points) {
  BPatch_image *appImage = app->getImage();
  BPatch_variableExpr *intCounter = app->malloc(*(appImage->findType("int")));
  BPatch_arithExpr addOne(BPatch_assign, *intCounter,
			  BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(1)));

  app->insertSnippet(addOne, *points);
}

// Example 4: finish things up (continue or write out)
void finishInstrumenting(BPatch_addressSpace *app, const char *newName) {
  BPatch_process *appProc = dynamic_cast<BPatch_process *>(app);
  BPatch_binaryEdit *appBin = dynamic_cast<BPatch_binaryEdit *>(app);

  if (appProc) {
    appProc->continueExecution();
    while (!appProc->isTerminated()) {
      bpatch.waitForStatusChange();
    }
  }
  if (appBin) {
    appBin->writeFile(newName);
  }
}

// Example 5: binary analysis
int binaryAnalysis(BPatch_addressSpace *app) {
  BPatch_image *appImage = app->getImage();
  int insns_access_memory = 0;

  std::vector<BPatch_function *> funcs;
  appImage->findFunction("InterestingProcedure", funcs);
  
  BPatch_flowGraph *fg = funcs[0]->getCFG();
  std::set<BPatch_basicBlock *> blocks;
  fg->getAllBasicBlocks(blocks);

  std::set<BPatch_basicBlock *>::iterator block_iter;
  for (block_iter = blocks.begin(); block_iter != blocks.end(); ++block_iter) { 
    BPatch_basicBlock *block = *block_iter;
    std::vector<Dyninst::InstructionAPI::Instruction::Ptr> insns;
    block->getInstructions(insns);
    
    std::vector<Dyninst::InstructionAPI::Instruction::Ptr>::iterator insn_iter;
    for (insn_iter = insns.begin(); insn_iter != insns.end(); ++insn_iter) {
      Dyninst::InstructionAPI::Instruction::Ptr insn = *insn_iter;
      if (insn->readsMemory() || insn->writesMemory()) {
	insns_access_memory++;
      }
    }
  }
  return insns_access_memory;
}

// Example 6: memory instrumentation
void instrumentMemory(BPatch_addressSpace *app) {
  BPatch_image *appImage = app->getImage();

  // We're interested in loads and stores
  BPatch_Set<BPatch_opCode> accessTypes;
  accessTypes.insert(BPatch_opLoad);
  accessTypes.insert(BPatch_opStore);

  // Get points for each load and store
  std::vector<BPatch_function *> funcs;
  appImage->findFunction("InterestingProcedure", funcs);
  std::vector<BPatch_point *> *points = funcs[0]->findPoint(accessTypes);
  
  // Create a snippet that calls printf with each effective address
  std::vector<BPatch_snippet *> printfArgs;
  BPatch_snippet *fmt = new BPatch_constExpr("Access at: %p\n");
  printfArgs.push_back(fmt);
  BPatch_snippet *eae = new BPatch_effectiveAddressExpr;
  printfArgs.push_back(eae);
  std::vector<BPatch_function *> printfFuncs;
  appImage->findFunction("printf", printfFuncs);
  BPatch_funcCallExpr printfCall(*(printfFuncs[0]), printfArgs);
  
  app->insertSnippet(printfCall, *points);
}

int main() {
  const char *progName = "InterestingProgram"; // = ...
  int progPID = 42; // = ...
  const char *progArgv[] = {"InterestingProgram", "-h", NULL}; // = ...

  // Example 1: create/attach/open a binary
  BPatch_addressSpace *app = startInstrumenting(create, // or attach or open
						progName,
						progPID,
						progArgv);

  // Example 2: get entry point
  std::vector<BPatch_point *> *entryPoint = findEntryPoint(app);
  
  // Example 3: create and insert increment snippet
  createAndInsertSnippet(app, entryPoint);

  // Example 4: finish up instrumenting
  finishInstrumenting(app, progName);

  // Example 5: get a count of memory accesses
  int insns_access_memory = binaryAnalysis(app);

  // Example 6: instrument memory accesses
  instrumentMemory(app);

  return 0;
}
