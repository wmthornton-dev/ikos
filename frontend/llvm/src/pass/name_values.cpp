/*******************************************************************************
 *
 * \file
 * \brief Implementation of the NameValuesPass
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

#include <ikos/frontend/llvm/pass.hpp>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace llvm;

namespace {

struct NameValuesPass final : public ModulePass {
  static char ID; // Pass identification

  unsigned global_idx;

  NameValuesPass() : ModulePass(ID), global_idx(0) {}

  void getAnalysisUsage(AnalysisUsage& AU) const override {
    AU.setPreservesAll();
  }

  bool runOnModule(Module& m) override {
    for (auto it = m.global_begin(), et = m.global_end(); it != et; ++it) {
      run_on_global(*it);
    }
    for (Function& f : m) {
      run_on_function(f);
    }
    return false;
  }

  bool run_on_global(GlobalVariable& g) {
    if (!g.hasName()) {
      g.setName("gv_" + std::to_string(++global_idx));
    }
    return false;
  }

  bool run_on_function(Function& f) {
    unsigned arg_idx = 1;
    for (auto it = f.arg_begin(), et = f.arg_end(); it != et; ++it, ++arg_idx) {
      Argument& a = *it;

      if (!a.hasName() && !a.getType()->isVoidTy()) {
        a.setName("arg_" + std::to_string(arg_idx));
      }
    }

    unsigned block_idx = 0;
    unsigned inst_idx = 0;
    for (BasicBlock& bb : f) {
      if (!bb.hasName()) {
        bb.setName("bb_" + std::to_string(++block_idx));
      }

      for (Instruction& i : bb) {
        if (!i.hasName() && !i.getType()->isVoidTy()) {
          i.setName("_" + std::to_string(++inst_idx));
        }
      }
    }

    return false;
  }

}; // end struct NameValuesPass

} // end anonymous namespace

char NameValuesPass::ID = 0;

INITIALIZE_PASS(
    NameValuesPass, "name-values", "Names all unnamed values", false, false);

ModulePass* ikos::frontend::pass::create_name_values_pass() {
  return new NameValuesPass();
}