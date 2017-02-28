#include <iostream>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/IO/reader/OBJReader.hh>
#include <OpenMesh/Core/IO/writer/OBJWriter.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>
#include <Eigen/Dense>
#include <stdio.h>
#include <RigidRegistration.hpp>
#include <NonrigidRegistration.hpp>
#include <InlierDetector.hpp>
#include <CorrespondenceFilter.hpp>
#include <SymmetricCorrespondenceFilter.hpp>
#include <RigidTransformer.hpp>
#include <ViscoElasticTransformer.hpp>
#include <Downsampler.hpp>
#include <ScaleShifter.hpp>
#include "global.hpp"
#include <helper_functions.hpp>

using namespace std;


typedef OpenMesh::DefaultTraits MyTraits;
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  TriMesh;
typedef OpenMesh::Decimater::DecimaterT<TriMesh>    DecimaterType;
typedef OpenMesh::Decimater::ModQuadricT<TriMesh>::Handle HModQuadric;
typedef Eigen::Matrix< int, Eigen::Dynamic, 3> FacesMat; //matrix Mx3 of type unsigned int
typedef Eigen::VectorXf VecDynFloat;
typedef Eigen::Matrix< float, Eigen::Dynamic, registration::NUM_FEATURES> FeatureMat; //matrix Mx6 of type float

int main()
{

    /*
    ############################################################################
    ##############################  INPUT  #####################################
    ############################################################################
    */
    //# IO variables
//    const string fuckedUpBunnyDir = "/home/jonatan/meshmonk/examples/data/bunny_slightly_rotated.obj";
    const string fuckedUpBunnyDir = "/home/jonatan/projects/meshmonk/examples/data/fucked_up_bunny.obj";
//    const string fuckedUpBunnyDir = "/home/jonatan/projects/meshmonk/examples/data/harry/cropped_face_template.obj";
    const string bunnyDir = "/home/jonatan/projects/meshmonk/examples/data/bunny90.obj";
//    const string bunnyDir = "/home/jonatan/projects/meshmonk/examples/data/harry/HM_cleaned_properly.obj";
    const string fuckedUpBunnyResultDir = "/home/jonatan/projects/meshmonk/examples/data/bunnyNonRigid.obj";
//    const string fuckedUpBunnyDir = "/home/jonatan/projects/meshmonk/examples/data/kul_gezichten/Outliers/Alspac/4707_template.obj";
//    const string bunnyDir = "/home/jonatan/projects/meshmonk/examples/data/kul_gezichten/Outliers/Alspac/4707_mevislab.obj";
//    const string fuckedUpBunnyResultDir = "/home/jonatan/projects/meshmonk/examples/data/kul_gezichten/Outliers/Alspac/4707_Monk.obj";

    //# Load meshes and convert to feature matrices
    FeatureMat originalFloatingFeatures;
    FeatureMat originalTargetFeatures;
    FacesMat originalFloatingFaces;
    FacesMat originalTargetFaces;
    registration::import_data(fuckedUpBunnyDir, bunnyDir,
                              originalFloatingFeatures, originalTargetFeatures,
                              originalFloatingFaces, originalTargetFaces);

    const size_t numFloatingVertices = originalFloatingFeatures.rows();
    const size_t numTargetVertices = originalTargetFeatures.rows();
    VecDynFloat originalFloatingFlags = VecDynFloat::Ones(numFloatingVertices);
    VecDynFloat originalTargetFlags = VecDynFloat::Ones(numTargetVertices);








//    //# Downsample the original Mesh, call it downsampleMesh
//    registration::Downsampler downsampler;
//    VecDynInt downOriginalIndices;
//    FeatureMat downFeatures;
//    FacesMat downFaces;
//    VecDynFloat downFlags;
//    downsampler.set_input(&floatingFeatures, &floatingFaces, &originalFloatingFlags);
//    downsampler.set_output(downFeatures, downFaces, downFlags, downOriginalIndices);
//    downsampler.set_parameters(0.5f);
//    downsampler.update();
//
//    //# Register it to the target mesh
//    size_t numNonrigidIterations = 20;
//    float sigmaSmoothing = 2.0f;
//    size_t numTargetVertices = targetFeatures.rows();
//    VecDynFloat targetFlags = VecDynFloat::Ones(numTargetVertices);
//    registration::NonrigidRegistration nonrigidRegistration;
//    nonrigidRegistration.set_input(&downFeatures, &targetFeatures, &downFaces, &downFlags, &targetFlags);
//    nonrigidRegistration.set_parameters(true, 5, 3.0,
//                                        numNonrigidIterations, sigmaSmoothing,
//                                        100, 1,
//                                        100, 1);
//    nonrigidRegistration.update();
//
//
//
//    //# Downsample the original again, but leave it at a higher resolution. Call it upsampleMesh.
//    registration::Downsampler upsampler;
//    VecDynInt upOriginalIndices;
//    FeatureMat upFeatures;
//    FacesMat upFaces;
//    VecDynFloat upFlags;
//    upsampler.set_input(&floatingFeatures, &floatingFaces, &originalFloatingFlags);
//    upsampler.set_output(upFeatures, upFaces, upFlags, upOriginalIndices);
//    upsampler.set_parameters(0.3f);
//    upsampler.update();
//
//    //# Transfer the vertex positions of downsampleMesh to upsampleMesh using ScaleShifter class
//    registration::ScaleShifter scaleShifter;
//    scaleShifter.set_input(downFeatures, downOriginalIndices, upOriginalIndices);
//    scaleShifter.set_output(upFeatures);
//    scaleShifter.update();


    int numPyramidLayers = 4;
    int floatPyramidStart = 70;//percentage you want to start downsampling the floating mesh by
    int targetPyramidStart = 70;//percentage you want to start downsampling the target mesh by
    int floatPyramidEnd = 0;
    int targetPyramidEnd = 0;
    size_t numNonrigidIterations = 21;
    //# Initialize the floating features, their original indices and the faces.
    /*
    We need to do this before the pyramid iterations, because those have to be passed from the
    previous iteration to the next one. The 'ScaleShifter' class makes sure that the properties
    of the floating mesh of the previous pyramid scale are transferred to the current pyramid
    scale.
    */
    FeatureMat floatingFeatures;
    FacesMat floatingFaces;
    VecDynInt floatingOriginalIndices;
    for (int i = 0 ; i < numPyramidLayers ; i++){
        //# Copy the floating features and indices of the previous pyramid scale
        FeatureMat oldFloatingFeatures;
        VecDynInt oldFloatingOriginalIndices;
        if (i > 0) {
            oldFloatingFeatures = FeatureMat(floatingFeatures);
            oldFloatingOriginalIndices = VecDynInt(floatingOriginalIndices);
        }

        //# Downsample Floating Mesh
        float downsampleRatio = float(std::round(floatPyramidEnd + std::round((floatPyramidStart-floatPyramidEnd)/numPyramidLayers) * (numPyramidLayers-i-1)));
        downsampleRatio /= 100.0f;
        std::cout << "Downsample Ratio Float : " << downsampleRatio << std::endl;
        registration::Downsampler downsampler;
        VecDynFloat floatingFlags;
        downsampler.set_input(&originalFloatingFeatures, &originalFloatingFaces, &originalFloatingFlags);
        downsampler.set_output(floatingFeatures, floatingFaces, floatingFlags, floatingOriginalIndices);
        downsampler.set_parameters(downsampleRatio);
        downsampler.update();

        //# Downsample Target Mesh
        downsampleRatio = float(std::round(targetPyramidEnd + std::round((targetPyramidStart-targetPyramidEnd)/numPyramidLayers) * (numPyramidLayers-i-1)));
        downsampleRatio /= 100.0f;
        std::cout << "Downsample Ratio target : " << downsampleRatio << std::endl;
        FeatureMat targetFeatures;
        FacesMat targetFaces;
        VecDynFloat targetFlags;
        downsampler.set_input(&originalTargetFeatures, &originalTargetFaces, &originalTargetFlags);
        downsampler.set_output(targetFeatures, targetFaces, targetFlags);
        downsampler.set_parameters(downsampleRatio);
        downsampler.update();

        //# Transfer floating mesh properties from previous pyramid scale to the current one.
        if (i > 0) {
            //## Scale up
            registration::ScaleShifter scaleShifter;
            scaleShifter.set_input(oldFloatingFeatures, oldFloatingOriginalIndices, floatingOriginalIndices);
            scaleShifter.set_output(floatingFeatures);
            scaleShifter.update();
        }

        //# Registration
        size_t iterations = int(std::floor(float(numNonrigidIterations)/float(numPyramidLayers) + 1.0f));
        float sigmaSmoothing = 2.0f;
        size_t numTargetVertices = targetFeatures.rows();
        registration::NonrigidRegistration nonrigidRegistration;
        nonrigidRegistration.set_input(&floatingFeatures, &targetFeatures, &floatingFaces, &floatingFlags, &targetFlags);
        nonrigidRegistration.set_parameters(true, 5, 3.0,
                                            iterations, sigmaSmoothing,
                                            100, 1,
                                            100, 1);
        nonrigidRegistration.update();
    }







//    floatingFeatures = FeatureMat::Zero(8, NUM_FEATURES);
//    targetFeatures = FeatureMat::Zero(8, NUM_FEATURES);
//    floatingFeatures << 0.1f, 0.1f, 0.1f, 0.0f, 0.0f, 1.0f,
//                        0.1f, 1.1f, 0.1f, 0.0f, 0.0f, 1.0f,
//                        1.1f, 0.1f, 0.1f, 0.0f, 0.0f, 1.0f,
//                        1.1f, 1.1f, 0.1f, 0.0f, 0.0f, 1.0f,
//                        0.1f, 0.1f, 1.1f, 0.0f, 0.0f, 1.0f,
//                        0.1f, 1.1f, 1.1f, 0.0f, 0.0f, 1.0f,
//                        1.1f, 0.1f, 1.1f, 0.0f, 0.0f, 1.0f,
//                        1.1f, 1.1f, 1.1f, 0.0f, 0.0f, 1.0f;
//    targetFeatures << 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
//                      0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
//                      1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
//                      1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
//                      0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//                      0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//                      1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
//                      1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f;




     /*
    ############################################################################
    ##############################  TEST SHIZZLE  ##############################
    ############################################################################
    */

//    std::cout << "test features: \n" << floatingFeatures.topRows(10) << std::endl;
//    std::cout << "test faces: \n" << floatingFaces.topRows(10) << std::endl;



    /*
    ############################################################################
    ##############################  DECIMATION  ################################
    ############################################################################
    */

//    //# Load the bunny
//    TriMesh fuckedUpBunny;
//    OpenMesh::IO::_OBJReader_();
//    if (!OpenMesh::IO::read_mesh(fuckedUpBunny,fuckedUpBunnyDir)){
//        std::cerr << "Read error \n";
//        exit(1);
//    };
//
//    //# Print Mesh property
//    std::cout << "--------  Before processing " << std::endl;
//    std::cout << "# Vertices " << fuckedUpBunny.n_vertices() << std::endl;
//    std::cout << "# Edges " << fuckedUpBunny.n_edges() << std::endl;
//    std::cout << "# Faces " << fuckedUpBunny.n_faces() << std::endl;
//
//    //# block boundary vertices
//    fuckedUpBunny.request_vertex_status();
//    //## Get an iterator over all halfedges
//    TriMesh::HalfedgeIter he_it, he_end=fuckedUpBunny.halfedges_end();
//    //## If halfedge is boundary, lock the corresponding vertices
//    for (he_it = fuckedUpBunny.halfedges_begin(); he_it != he_end ; ++he_it) {
//      if (fuckedUpBunny.is_boundary(*he_it)) {
//         fuckedUpBunny.status(fuckedUpBunny.to_vertex_handle(*he_it)).set_locked(true);
//         fuckedUpBunny.status(fuckedUpBunny.from_vertex_handle(*he_it)).set_locked(true);
//      }
//    }
//    //# Make sure mesh has necessary normals etc
//    fuckedUpBunny.request_face_normals();
//    fuckedUpBunny.update_face_normals();
//
//    //# Set up the decimator
//    DecimaterType decimater(fuckedUpBunny);  // a decimater object, connected to a mesh
//    HModQuadric hModQuadric;      // use a quadric module
//    bool addSucces = decimater.add( hModQuadric ); // register module at the decimater
//    std::cout << "Adding quadric modifier to decimater : " << addSucces << std::endl;
//
//    std::cout << decimater.module( hModQuadric ).name() << std::endl;
//    decimater.module(hModQuadric).unset_max_err();
//
//    //# Initialize the decimater
//    bool rc = decimater.initialize();
//    std::cout  << "Decimater Initialization: " << decimater.is_initialized() << std::endl;
//    if (!rc){
//        std::cerr << "  initializing failed!" << std::endl;
//        std::cerr << "  maybe no priority module or more than one were defined!" << std::endl;
//        return false;
//    }
//
//
//    std::cout << "Decimater Observer: " << decimater.observer() << std::endl;
//
//
//
//    //# Run the decimater
////    rc = decimater.decimate_to(size_t(10));
//    rc = decimater.decimate(size_t(10));
//    std::cout << " Did we fucking decimate?? -> " << rc << std::endl;
//
//    //# Collect garbage
//    fuckedUpBunny.garbage_collection();
//
//    //# Print Mesh property
//    std::cout << "--------  After processing " << std::endl;
//    std::cout << "# Vertices " << fuckedUpBunny.n_vertices() << std::endl;
//    std::cout << "# Edges " << fuckedUpBunny.n_edges() << std::endl;
//    std::cout << "# Faces " << fuckedUpBunny.n_faces() << std::endl;
//
//
//
//    //# Write to file
//    OpenMesh::IO::_OBJWriter_();
//    if (!OpenMesh::IO::write_mesh(fuckedUpBunny, fuckedUpBunnyResultDir))
//    {
//        std::cerr << "write error\n";
//        exit(1);
//    }
//
//    //# convert to our features
//    registration::convert_mesh_to_matrices(fuckedUpBunny, floatingFeatures, floatingFaces);


    /*
    ############################################################################
    ##############################  RIGID ICP  #################################
    ############################################################################
    */

//    //# Info & Initialization
//    //## Data and matrices
//    size_t numFloatingVertices = floatingFeatures.rows();
//    size_t numTargetVertices = targetFeatures.rows();
//    VecDynFloat floatingFlags = VecDynFloat::Ones(numFloatingVertices);
//    VecDynFloat targetFlags = VecDynFloat::Ones(numTargetVertices);
//
//    //## Parameters
//    bool symmetric = true;
//    size_t numNearestNeighbours = 5;
//    size_t numRigidIterations = 10;
//    float kappa = 3.0f;
//
//    registration::RigidRegistration rigidRegistration;
//    rigidRegistration.set_input(&floatingFeatures, &targetFeatures, &floatingFlags, &targetFlags);
//    rigidRegistration.set_parameters(symmetric, numNearestNeighbours, kappa, numRigidIterations);
//    rigidRegistration.update();
//
//    /*
//    ############################################################################
//    ##########################  NON-RIGID ICP  #################################
//    ############################################################################
//    */
//    //## Initialization
//    size_t numNonrigidIterations = 20;
//    float sigmaSmoothing = 2.0f;
//    registration::NonrigidRegistration nonrigidRegistration;
//    nonrigidRegistration.set_input(&floatingFeatures, &targetFeatures, &floatingFaces, &floatingFlags, &targetFlags);
//    nonrigidRegistration.set_parameters(symmetric, numNearestNeighbours, kappa,
//                                        numNonrigidIterations, sigmaSmoothing,
//                                        100, 1,
//                                        100, 1);
//    nonrigidRegistration.update();

    /*
    ############################################################################
    ##############################  OUTPUT #####################################
    ############################################################################
    */
    //# Write result to file
//    registration::export_data(floatingFeatures,floatingFaces, fuckedUpBunnyResultDir);
    registration::export_data(floatingFeatures,floatingFaces, fuckedUpBunnyResultDir);
//    registration::export_data(downFeatures,downFaces, "/home/jonatan/projects/meshmonk/examples/data/bunnyDown.obj");



    return 0;
}
