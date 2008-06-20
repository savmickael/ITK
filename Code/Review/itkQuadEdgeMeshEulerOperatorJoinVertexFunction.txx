/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkQuadEdgeMeshEulerOperatorJoinVertexFunction.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkQuadEdgeMeshEulerOperatorJoinVertexFunction_txx
#define __itkQuadEdgeMeshEulerOperatorJoinVertexFunction_txx

#include "itkQuadEdgeMeshEulerOperatorJoinVertexFunction.h"
#include "itkQuadEdgeMeshZipMeshFunction.h"

#include <list>
#include <algorithm>

namespace itk
{

template < class TMesh, class TQEType >
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
QuadEdgeMeshEulerOperatorJoinVertexFunction() : Superclass(),
  m_OldPointID( 0 ), m_EdgeStatus( 0 )
{}

//--------------------------------------------------------------------------
template < class TMesh, class TQEType >
typename QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::OutputType
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
Evaluate( QEType* e )
{
  std::stack< QEType* > edges_to_be_deleted;
  m_EdgeStatus = CheckStatus( e, edges_to_be_deleted );

  switch( m_EdgeStatus )
  {
    default:
    case 0:
      return Process( e );
    case 1: // e == 0
    case 2: // m_Mesh == 0
    case 3: // e->IsIsolated() && e_sym->IsIsolated()
    case 4: // more than 2 common vertices in 0-ring of org and dest respectively
    case 5: // Tetraedron case
      return( (QEType*) 0 );
    case 6: // Isolated edge
      return ProcessIsolatedQuadEdge( e );
    case 7: // Isolated face
      return ProcessIsolatedFace( e, edges_to_be_deleted );
    case 8: // Samosa case
    case 9: // Eye case
      return( (QEType*) 0 );
  }
}
//--------------------------------------------------------------------------
template< class TMesh, class TQEType >
TQEType*
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
Process( QEType* e )
{
  QEType* e_sym = e->GetSym();

//   bool IsEdgeIsolated = e->IsIsolated( );
//   bool IsSymEdgeIsolated = e_sym->IsIsolated( );

  // General case
  bool wasLeftFace     = e->IsLeftSet( );
  bool wasRiteFace     = e->IsRightSet( );
  bool wasLeftTriangle = e->IsLnextOfTriangle( );
  bool wasRiteTriangle = e_sym->IsLnextOfTriangle( );

  PointIdentifier NewDest = e->GetDestination( );
  PointIdentifier NewOrg  = e->GetOrigin( );
  QEType* leftZip = e->GetLnext( );
  QEType* riteZip = e->GetOprev( );


  //
  //                    \   |   /                //
  //                     \  |  /                 //
  //                      \ | /                  //
  //     ------------------ b ------------- Y    //
  //                   ___/ |               |    //
  //      _<_leftZip__/     |               |    //
  //     /                  ^               |    //
  //    X      left         e     rite      |    //
  //     \____________      |               |    //
  //                  \___  |               |    //
  //                      \ |               |    //
  //     ------------------ a --riteZip->-- Y    //
  //                      / | \                  //
  //                     /  |  \                 //
  //                    /   |   \                //
  //
  this->m_Mesh->LightWeightDeleteEdge( e );
  this->m_OldPointID = this->m_Mesh->Splice( leftZip, riteZip );
 
  //
  //                            |      /       __Y  //
  //                            |     /     __/  |  //
  //       __<_leftZip___       |    /   __/     |  //
  //      /              \      |   / __/        |  //
  //     /                \__   |  / /           |  //
  //    X      NOFACE      __ [a = b]    NOFACE  |  //
  //     \                /  / / | \ \__         |  //
  //      \______________/ _/ /  |  \   \__      |  //
  //                    __/  /   |   \  riteZip  |  //
  //                 __/    /    |    \       \__|  //
  //                /      /     |     \         Y  //
  // 
  // When the Lnext and/or the Rnext ring of the argument edge was originaly
  // the one[s] of a triangle, the above edge deletion created the odd
  // situation of having two different edges adjacent to the same two
  // vertices (which is quite a bad thing). This is was is depicted on 
  // the above ascii-graph, the original left face was a triangle and
  // the resulting situation has two different edges adjacent to the
  // two vertices X and a. In order to clean up things, we can call the
  // Zip(MeshFunction) algorithm which handles this case.
  // Once things are back to normal, we recreate faces when they were
  // originaly present.
  //

  typedef QuadEdgeMeshZipMeshFunction< MeshType, QEType > Zip;
  if( wasLeftTriangle )
    {
    typename Zip::Pointer zip = Zip::New( );
    zip->SetInput( this->m_Mesh );
    if( QEType::m_NoPoint != zip->Evaluate( leftZip ) )
      {
      itkDebugMacro( "Zip must return NoPoint (left)." );
      return( (QEType*) 0 );
      }
    }
  else
    {
    if( wasLeftFace )
      {
      this->m_Mesh->AddFace( leftZip );
      }
    }

  // NewOrg = riteZip->GetOprev( )->GetDestination( );
  if( wasRiteTriangle )
    {
    NewOrg = riteZip->GetDestination( );
    typename Zip::Pointer zip = Zip::New( );
    zip->SetInput( this->m_Mesh );
    if( QEType::m_NoPoint != zip->Evaluate( riteZip ) )
      {
      itkDebugMacro( "Zip must return NoPoint (right)." );
      return( (QEType*) 0 );
      }
    }
  else
    {
    NewOrg = riteZip->GetLprev( )->GetOrigin( );
    if( wasRiteFace )
      {
      this->m_Mesh->AddFace( riteZip );
      }
    }
  
  OutputType result = this->m_Mesh->FindEdge( NewOrg, NewDest );
  if( !result)
    {
    result = this->m_Mesh->FindEdge( NewDest )->GetSym( );
    } 
  return( result );
}

//--------------------------------------------------------------------------
template < class TMesh, class TQEType >
TQEType*
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
ProcessIsolatedQuadEdge( QEType* e )
{
  QEType* temp = ( e->IsIsolated() == true ) ? e->GetSym() : e;
  QEType* rebuildEdge = temp->GetOprev();
  m_OldPointID = temp->GetSym()->GetOrigin();

  bool e_leftset = e->IsLeftSet( );
  this->m_Mesh->LightWeightDeleteEdge( e );
  if( e_leftset )
    {
    this->m_Mesh->AddFace( rebuildEdge );
    }
      
  // this case has no symetric case in SPlitVertex
  // i.e. it is impossible to reconstruct such a pathological
  // case using SplitVertex. Thus the return value is
  // of less interest.
  // We return an edge whose dest is a, whichever.
  return( rebuildEdge );
}
//--------------------------------------------------------------------------
template< class TMesh, class TQEType >
TQEType*
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
ProcessIsolatedFace( QEType* e, std::stack< QEType* > EdgesToBeDeleted )
{
  PointIdentifier org = e->GetOrigin();
  PointIdentifier dest = e->GetDestination();

  // delete all elements
  while( !EdgesToBeDeleted.empty() )
    {
    this->m_Mesh->LightWeightDeleteEdge( EdgesToBeDeleted.top() );
    EdgesToBeDeleted.pop();
    }
  // it now retuns one edge from NewDest or NewOrg if there are any
  // else NULL
  QEType* temp = this->m_Mesh->FindEdge( dest );
  if( temp != 0 )
    {
    return temp;
    }
  else
    {
    return this->m_Mesh->FindEdge( org );
    }
}

//--------------------------------------------------------------------------
template < class TMesh, class TQEType >
bool
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
IsFaceIsolated( QEType* e,
  const bool& iWasLeftFace,
  std::stack< TQEType* >& oToBeDeleted )
{
  bool border;
  QEType* e_sym = e->GetSym();

  // turn around the face (left or right one) while edges are on the border
  // and push them into a stack (which will be used to delete properly all
  // elements )
  QEType* temp = ( iWasLeftFace == true ) ? e : e_sym;
  QEType* e_it = temp;

  oToBeDeleted.push( e_it );
  e_it = e_it->GetLnext();
    
  do
    {
    oToBeDeleted.push( e_it );
    border = e_it->IsAtBorder();
    e_it = e_it->GetLnext();
    } while( ( e_it != temp ) && border );

  return border;
}
//--------------------------------------------------------------------------
template< class TMesh, class TQEType >
unsigned int
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
CheckStatus( QEType* e, std::stack< TQEType* >& oToBeDeleted )
{
  if( !e )
    {
    itkDebugMacro( "Input is not an edge." );
    return 1;
    }

  if( !this->m_Mesh )
    {
    itkDebugMacro( "No mesh present." );
    return 2;
    }

  QEType* e_sym = e->GetSym();

  bool IsEdgeIsolated = e->IsIsolated( );
  bool IsSymEdgeIsolated = e_sym->IsIsolated( );

  if( IsEdgeIsolated && IsSymEdgeIsolated )
    {
    // We could shrink the edge to a point,
    // But we consider this case to be degenerated.
    itkDebugMacro( "Argument edge isolated." );
    return 3;
    }

  size_t number_common_vertices = CommonVertexNeighboor( e );
  if( number_common_vertices > 2 )
    {
    itkDebugMacro("The 2 vertices have more than 2 common neighboor vertices.");
    return 4;
    }

  if( number_common_vertices == 2 )
    {
    if( IsTetraedron( e ) )
      {
      itkDebugMacro( "It forms a tetraedron." );
      return 5;
      }
    }

  if( IsEdgeIsolated || IsSymEdgeIsolated )
    {
    // One the endpoints (and only one) of the incoming edge is isolated.
    // Instead of "shrinking" the edge it suffice to delete it. Note that
    // this also avoids trouble since the definition of leftZip or riteZip
    // would be improper in this case (in fact leftZip or riteZip would
    // in fact be e or e->GetSym( )...
    //
    // When e is adjacent to a face, we must retrieve the edge [ a, b ]
    // in order to rebuild the face after edge deletion. If the left face
    // of e is set then the right face is also set... since it is the same
    // face ! Here is the situation:
    //
    //        b----a----X
    //        |    |    |
    //        |    e    |
    //        |    |    |
    //        |         |
    //        X----X----X
    //
    // We are not yet sure of the orientation of e and which endpoint
    // of e is attached in a.
    return 6;
    }

  // General case
  bool wasLeftFace     = e->IsLeftSet( );
  bool wasRiteFace     = e->IsRightSet( );
//   bool wasLeftTriangle = e->IsLnextOfTriangle( );
//   bool wasRiteTriangle = e_sym->IsLnextOfTriangle( );

  // case where the edge belongs to an isolated polygon
  // Is edge at the border
  if( ( wasLeftFace && !wasRiteFace ) || ( !wasLeftFace && wasRiteFace ) )
    {
    if( IsFaceIsolated( e, wasLeftFace, oToBeDeleted ) )
      {
      return 7;
      }
    }

  if( IsSamosa( e ) )
    {
    return 8;
    }
  if( IsEye( e ) )
    {
    return 9;
    }


  return 0;
}
//--------------------------------------------------------------------------
template < class TMesh, class TQEType >
bool
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
IsTetraedron( QEType* e )
{
  QEType* e_sym = e->GetSym();

  if( ( e->GetOrder() == 3 ) &&
      ( e_sym->GetOrder() == 3 ) &&
      ( e->GetLprev()->GetOrder() == 3 ) &&
      ( e_sym->GetLprev()->GetOrder() == 3 ) )
    {
    bool left_triangle = e->IsLnextOfTriangle( );
    bool right_triangle = e_sym->IsLnextOfTriangle( );
  
    if( left_triangle && right_triangle )
      {
      CellIdentifier id_left_right_triangle;
      if( e->GetLprev()->IsRightSet() )
        {
        id_left_right_triangle = e->GetLprev()->GetRight();
        }
      else
        {
        return false;
        }

      CellIdentifier id_left_left_triangle;
      if( e->GetLnext()->IsRightSet() )
        {
        id_left_left_triangle = e->GetLnext()->GetRight();
        }
      else
        {
        return false;
        }

      CellIdentifier id_right_left_triangle;
      if( e_sym->GetLnext()->IsRightSet() )
        {
        id_right_left_triangle = e_sym->GetLnext()->GetRight();
        }
      else
        {
        return false;
        }

      CellIdentifier id_right_right_triangle;
      if( e_sym->GetLprev()->IsRightSet() )
        {
        id_right_right_triangle = e_sym->GetLprev()->GetRight();
        }
      else
        {
        return false;
        }

      if( ( id_left_right_triangle == id_right_left_triangle ) &&
          ( id_left_left_triangle == id_right_right_triangle ) )
        {
        return true;
        }
      }
    }

  return false;
}
//--------------------------------------------------------------------------
template < class TMesh, class TQEType >
bool
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
IsSamosa( QEType* e )
{
  return ( ( e->GetOrder() == 2 ) &&  ( e->GetSym()->GetOrder() == 2 ) );
}

//--------------------------------------------------------------------------
template < class TMesh, class TQEType >
bool
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
IsEye( QEType* e )
{
  bool OriginOrderIsTwo = ( e->GetOrder( ) == 2 );
  bool DestinationOrderIsTwo = ( e->GetSym()->GetOrder( ) == 2 );

  return ( ( OriginOrderIsTwo && !DestinationOrderIsTwo ) ||
           ( !OriginOrderIsTwo && DestinationOrderIsTwo ) );
}

template < class TMesh, class TQEType >
void
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "OldPointID: " << this->m_OldPointID << std::endl;
  os << indent << "EdgeStatus: " << this->m_EdgeStatus << std::endl;
}

//--------------------------------------------------------------------------
template < class TMesh, class TQEType >
size_t
QuadEdgeMeshEulerOperatorJoinVertexFunction< TMesh, TQEType >::
CommonVertexNeighboor( QEType* e )
{
///NOTE  arnaud gelas: these first tests can not be applied in the
///  case of mesh with boundaries (so I commented out for the time being)
// 
//   bool isLeftTriangle = e->IsLnextOfTriangle( );
//   bool isRiteTriangle = e_sym->IsLnextOfTriangle( );
// 
//   //easy case
//   if( isLeftTriangle && isRiteTriangle )
//     {
//     if( e->GetOrder( ) <= 3 && e_sym->GetOrder( ) <= 3 )
//       return( true );
//     if( e->GetOnext( )->GetSym( )->GetOrder( ) <= 3 )
//       return( true );
//     if( e->GetOprev( )->GetSym( )->GetOrder( ) <= 3 )
//       return( true );
//     }
// 
//   // general case

///NOTE arnaud gelas: I replace the previous code by this one which is cleaner
  QEType* qe = e;
  QEType* e_it  = qe->GetOnext( );

  typedef std::list< PointIdentifier > PointIdentifierList;
  PointIdentifierList dir_list;
  PointIdentifierList sym_list;
  PointIdentifierList intersection_list;

  PointIdentifier id;
  do
  {
    id = e_it->GetDestination();
    dir_list.push_back( id );
    e_it = e_it->GetOnext();
  } while( e_it != qe );

  qe = qe->GetSym();
  e_it = qe;

  do
  {
    id = e_it->GetDestination();
    sym_list.push_back( id );
    e_it = e_it->GetOnext();
  } while( e_it != qe );

  dir_list.sort();
  sym_list.sort();

  std::set_intersection( dir_list.begin(), dir_list.end(),
    sym_list.begin(), sym_list.end(),
    std::back_inserter( intersection_list ) );

  return intersection_list.size();

//   unsigned int counter = 0;
//   QEType* e_it = e->GetOnext( );
//   
//   while( e_it != e )
//     {
//     QEType* e_sym_it = e_sym->GetOnext( );
//     while( e_sym_it != e_sym )
//       {
//       if(  e_it->GetDestination() == e_sym_it->GetDestination() )
//         counter++;
//       e_sym_it = e_sym_it->GetOnext( );
//       }
//     e_it = e_it->GetOnext( );
//     }
//   return ( counter > 2 );
}

} // namespace itkQE

#endif

// eof - itkQuadEdgeMeshEulerOperatorJoinVertexFunction.txx
