/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "reaction/query_reaction.h"
#include "molecule/query_molecule.h"
#include "molecule/elements.h"

using namespace indigo;

QueryReaction::QueryReaction ()
{
}

QueryReaction::~QueryReaction ()
{
}

void QueryReaction::clear ()
{
   BaseReaction::clear();
}

QueryMolecule & QueryReaction::getQueryMolecule (int index)
{
   return (QueryMolecule &)getBaseMolecule(index);
}

Array<int> & QueryReaction::getExactChangeArray (int index)
{
   return _exactChanges[index];
}

int QueryReaction::getExactChange (int index, int atom)
{
   return _exactChanges[index][atom];
}

void QueryReaction::_addedBaseMolecule (int idx, int side, BaseMolecule &mol)
{
   BaseReaction::_addedBaseMolecule(idx, side, mol);
   _exactChanges.expand(idx + 1);
   _exactChanges[idx].clear_resize(mol.vertexEnd());
   _exactChanges[idx].zerofill();
}

void QueryReaction::makeTransposedForSubstructure (QueryReaction &other)
{
   QS_DEF(Array<int>, transposition);

   clear();

   for (int i = other.begin(); i < other.end(); i = other.next(i)) {
      int index = _allMolecules.size();

      other._transposeMoleculeForSubstructure(i, transposition);
      _allMolecules.add(new QueryMolecule());

      QueryMolecule &qmol = *(QueryMolecule *)_allMolecules[index];

      qmol.makeSubmolecule(other.getQueryMolecule(i), transposition, 0);

      _addedBaseMolecule(index, other._indexes[i], qmol);

      for (int j = 0; j < transposition.size(); j++) {
         getAAMArray(index).at(j) = other.getAAM(i, transposition[j]);
         getInversionArray(index).at(j) = other.getInversion(i, transposition[j]);
         getExactChangeArray(index).at(j) = other.getExactChange(i, transposition[j]);
      }

      for (int j = qmol.edgeBegin(); j != qmol.edgeEnd(); j = qmol.edgeNext(j)) {
         const Edge &edge = getBaseMolecule(index).getEdge(j);
         int edge_idx = other.getBaseMolecule(i).findEdgeIndex(transposition[edge.beg], transposition[edge.end]);
         getReactingCenterArray(index).at(j) = other.getReactingCenter(i, edge_idx);
      }
   }

}

void QueryReaction::_transposeMoleculeForSubstructure (int index, Array<int> &transposition)
{
   QS_DEF(Array<int>, has_reacting_info);
   QueryMolecule &mol = *(QueryMolecule *)_allMolecules[index];

   Array<int> &aam = getAAMArray(index);
   Array<int> &rc = getReactingCenterArray(index);
   Array<int> &inv = getInversionArray(index);
   Array<int> &ex = getExactChangeArray(index);

   has_reacting_info.clear_resize(mol.vertexEnd());
   has_reacting_info.zerofill();
   transposition.clear();

   for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i)) {
      if (aam[i] > 0)
         has_reacting_info[i] += 4;
      if (inv[i] > 0 || ex[i] > 0)
         has_reacting_info[i] += 1;

      transposition.push(i);
   }

   for (int i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
      if (rc[i] > 0)
      {
         const Edge &edge = mol.getEdge(i);
         has_reacting_info[edge.beg] += 2;
         has_reacting_info[edge.end] += 2;
      }

   _SortingContext context(mol, has_reacting_info);

   transposition.qsort(_compare, &context);
}

int QueryReaction::_compare (int &i1, int &i2, void *c)
{
   _SortingContext &context = *(_SortingContext *) c;

   bool is_pseudo1 = context.m.isPseudoAtom(i1);
   bool is_pseudo2 = context.m.isPseudoAtom(i2);
   if (is_pseudo1 != is_pseudo2)
   {
      if (is_pseudo1)
         return -1;
      return 1;
   }

   // Compare by AAM, reacting centers and other reacting flags
   int res = context.rdata[i2] - context.rdata[i1];

   if (res != 0 || is_pseudo1)
      return res;

   // Compare by atom frequency
   int labels_by_freq[] = {ELEM_C, ELEM_H, ELEM_O, ELEM_N, ELEM_P, ELEM_F,
      ELEM_S, ELEM_Si, ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};

   // DPX: what if atom number is not exactly known (-1) ?
   int label1 = context.m.getAtomNumber(i1);
   int label2 = context.m.getAtomNumber(i2);
   int idx1, idx2;

   for (idx1 = 0; idx1 < (int) NELEM(labels_by_freq); idx1++)
      if (label1 == labels_by_freq[idx1])
         break;
   for (idx2 = 0; idx2 < (int) NELEM(labels_by_freq); idx2++)
      if (label2 == labels_by_freq[idx2])
         break;

   res = idx2 - idx1;

   if (res != 0)
      return res;

   // compare by degree
   return context.m.getVertex(i2).degree() - context.m.getVertex(i1).degree();
}

int QueryReaction::_addBaseMolecule (int side)
{
   int idx = _allMolecules.size();
   _allMolecules.add(new QueryMolecule());
   _addedBaseMolecule(idx, side, *_allMolecules[idx]);
   return idx;
}

bool QueryReaction::aromatize() {
   bool arom_found = false;
   for (int i = begin(); i < end(); i = next(i)) {
      arom_found |= QueryMoleculeAromatizer::aromatizeBonds(*(QueryMolecule *)_allMolecules[i]);
   }

   return arom_found;
}

bool QueryReaction::dearomatize() {
   throw Error("Dearomatization not implemented");
}

void QueryReaction::_clone (BaseReaction &other, int index, int i, ObjArray< Array<int> >* mol_mappings)
{
   BaseMolecule& rmol = other.getBaseMolecule(i);
   //for query
   getExactChangeArray(index).resize(other.asQueryReaction().getExactChangeArray(i).size());
   if(getExactChangeArray(index).size() > 0) {
      for(int j = rmol.vertexBegin(); j < rmol.vertexEnd(); j = rmol.vertexNext(j)) {
         getExactChangeArray(index).at(j) = other.asQueryReaction().getExactChange(i, mol_mappings->at(i)[j]);
      }
   }
}

QueryReaction & QueryReaction::asQueryReaction ()
{
   return *this;
}

bool QueryReaction::isQueryReaction ()
{
   return true;
}

BaseReaction * QueryReaction::neu ()
{
   return new QueryReaction();
}
