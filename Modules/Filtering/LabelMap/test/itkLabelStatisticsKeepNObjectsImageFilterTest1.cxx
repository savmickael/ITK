/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSimpleFilterWatcher.h"

#include "itkLabelStatisticsKeepNObjectsImageFilter.h"

#include "itkTestingMacros.h"

int itkLabelStatisticsKeepNObjectsImageFilterTest1(int argc, char * argv[])
{

  if( argc != 8 )
    {
    std::cerr << "Usage: " << argv[0] << " input feature output";
    std::cerr << " background numberOfObjectsToKeep";
    std::cerr << "reverseOrdering attribute" << std::endl;
    return EXIT_FAILURE;
    }

  const unsigned int dim = 2;

  typedef itk::Image< unsigned char, dim > IType;

  typedef itk::ImageFileReader< IType > ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[1] );

  ReaderType::Pointer reader2 = ReaderType::New();
  reader2->SetFileName( argv[2] );

  typedef itk::LabelStatisticsKeepNObjectsImageFilter< IType, IType > LabelKeepNObjectsType;
  LabelKeepNObjectsType::Pointer KeepNObjects = LabelKeepNObjectsType::New();

  KeepNObjects->SetInput( reader->GetOutput() );
  KeepNObjects->SetFeatureImage( reader2->GetOutput() );

  //testing get/set BackgroundValue macro
  int BackgroundValue = ( atoi(argv[4]) );
  KeepNObjects->SetBackgroundValue( BackgroundValue );
  TEST_SET_GET_VALUE( BackgroundValue, KeepNObjects->GetBackgroundValue() );

  //testing get and set macros for Lambda
  unsigned int numberOfObjects = atoi( argv[5] );
  KeepNObjects->SetNumberOfObjects( numberOfObjects );
  TEST_SET_GET_VALUE( numberOfObjects, KeepNObjects->GetNumberOfObjects() );

  //testing boolean macro for ReverseOrdering
  KeepNObjects->ReverseOrderingOn();
  TEST_SET_GET_VALUE( true, KeepNObjects->GetReverseOrdering() );

  KeepNObjects->ReverseOrderingOff();
  TEST_SET_GET_VALUE( false, KeepNObjects->GetReverseOrdering() );

  //testing get and set macros or ReverseOrdering
  bool reverseOrdering = atoi( argv[6] );
  KeepNObjects->SetReverseOrdering( reverseOrdering );
  TEST_SET_GET_VALUE( reverseOrdering , KeepNObjects->GetReverseOrdering() );

  //testing get and set macros for Attribute
  LabelKeepNObjectsType::AttributeType attribute = atoi( argv[7] );
  KeepNObjects->SetAttribute( attribute );
  TEST_SET_GET_VALUE( attribute, KeepNObjects->GetAttribute() );

  itk::SimpleFilterWatcher watcher(KeepNObjects, "filter");

  typedef itk::ImageFileWriter< IType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( KeepNObjects->GetOutput() );
  writer->SetFileName( argv[3] );
  writer->UseCompressionOn();

  TRY_EXPECT_NO_EXCEPTION( writer->Update() );

  std::cout << "Test Complete!" << std::endl;

  return EXIT_SUCCESS;
}
