// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <GL/glut.h>
#include <float.h>
#include "src/Vec3.h"
#include "src/Camera.h"

#include <map>
#include<math.h>


enum DisplayMode{ WIRE=0, SOLID=1, LIGHTED_WIRE=2, LIGHTED=3 };

struct Triangle {
    inline Triangle () {
        v[0] = v[1] = v[2] = 0;
    }
    inline Triangle (const Triangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
    }
    inline Triangle (unsigned int v0, unsigned int v1, unsigned int v2) {
        v[0] = v0;   v[1] = v1;   v[2] = v2;
    }
    unsigned int & operator [] (unsigned int iv) { return v[iv]; }
    unsigned int operator [] (unsigned int iv) const { return v[iv]; }
    inline virtual ~Triangle () {}
    inline Triangle & operator = (const Triangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
        return (*this);
    }
    // membres indices des sommets du triangle:
    unsigned int v[3];
};

struct Voxel {
    Vec3 position = Vec3(0, 0, 0);
    Vec3 normale = Vec3(0, 0, 0);
    int compteur = 0;
    int idR;
};

struct Grid {
    int resolution;
    //int mapSize = pow(resolution, 3);
    std::map <unsigned int, Voxel> representants;

};

struct Mesh {
    std::vector< Vec3 > vertices; //array of mesh vertices positions
    std::vector< Vec3 > normals; //array of vertices normals useful for the display
    std::vector< Triangle > triangles; //array of mesh triangles
    std::vector< Vec3 > triangle_normals; //triangle normals to display face normals

    Vec3 vecMin, vecMax;

    //Compute face normals for the display
    void computeTrianglesNormals(){

        //A faire : implémenter le calcul des normales par face
        //Attention commencer la fonction par triangle_normals.clear();
        triangle_normals.clear();

        //Iterer sur les triangles
        for (unsigned int i = 0; i < triangles.size(); i++) {

            //La normale du triangle i est le resultat du produit vectoriel de deux ses arêtes e_10 et e_20 normalisé (e_10^e_20)
            //L'arete e_10 est représentée par le vecteur partant du sommet 0 (triangles[i][0]) au sommet 1 (triangles[i][1])
            //L'arete e_20 est représentée par le vecteur partant du sommet 0 (triangles[i][0]) au sommet 2 (triangles[i][2])

            Vec3 s0 = vertices[triangles[i][0]];
            Vec3 s1 = vertices[triangles[i][1]];
            Vec3 s2 = vertices[triangles[i][2]];

            Vec3 e_10 = s1 - s0;
            Vec3 e_20 = s2 - s0;

            Vec3 produitVect = Vec3::cross(e_10, e_20);

            //Normaliser et ajouter dans triangle_normales
            produitVect.normalize();

            triangle_normals.push_back(produitVect);

        }

    }

    //Compute vertices normals as the average of its incident faces normals
    void computeVerticesNormals(int weight_type){
        //Utiliser weight_type : 0 uniforme, 1 aire des triangles, 2 angle du triangle

        //A faire : implémenter le calcul des normales par sommet comme la moyenne des normales des triangles incidents
        //Attention commencer la fonction par normals.clear();
        normals.clear();

        //Initializer le vecteur normals taille vertices.size() avec Vec3(0., 0., 0.)
        normals.resize(vertices.size());

        for (unsigned int i = 0; i < vertices.size(); i++) {
            normals[i] = Vec3(0., 0., 0.);
        }

        float weight = 0;
        float P, p;
        float x_10, y_10, z_10, x_20, y_20, z_20, x_21, y_21, z_21;
        float dist_10, dist_20, dist_21;
        float a, b, c;

        //Iterer sur les triangles

            for (unsigned int i = 0; i < triangles.size(); i++) {

                //Pour chaque triangle i
                //Ajouter la normale au triangle à celle de chacun des sommets en utilisant des poids
                //0 uniforme, 1 aire du triangle, 2 angle du triangle

                if (weight_type == 0) {
                    weight = 1;
                }
               else if (weight_type == 1 || weight_type == 2) {

                    Vec3 s0 = vertices[triangles[i][0]];
                    Vec3 s1 = vertices[triangles[i][1]];
                    Vec3 s2 = vertices[triangles[i][2]];

                    /*Vec3 e_10 = s1 - s0;
                    Vec3 e_20 = s2 - s0;*/

                    x_10 = pow((s1[0]-s0[0]), 2);
                    y_10 = pow((s1[1]-s0[1]), 2);
                    z_10 = pow((s1[2]-s0[2]), 2);

                    x_21= pow((s2[0]-s1[0]), 2);
                    y_21 = pow((s2[1]-s1[1]), 2);
                    z_21 = pow((s2[2]-s1[2]), 2);

                    x_20= pow((s0[0]-s2[0]), 2);
                    y_20 = pow((s0[1]-s2[1]), 2);
                    z_20 = pow((s0[2]-s2[2]), 2);

                    dist_10 = sqrt((x_10 + y_10 + z_10));
                    dist_21 = sqrt(x_21 + y_21 + z_21);
                    dist_20 = sqrt(x_20 + y_20 + z_20);

                    if (weight_type == 1) {
                        P = dist_10 + dist_21 + dist_20;

                        p = P/2;

                        weight = sqrt(p*(p-dist_10)*(p-dist_21)*(p-dist_20));
                        // ou avec le cross product : weight = Vec3::cross(e_10, e_20)/2;
                    }

                    else {
                        a = pow(dist_10, 2);
                        b = pow(dist_21, 2);
                        c = pow(dist_20, 2);

                        if (i == triangles[i][0]) {
                            weight = acos((b + c - a) / 2*b*c);
                        }
                        else if (i == triangles[i][1]) {
                            weight = acos((a + c - b) / 2*a*c);
                        }
                        else {
                            weight = acos((a + b - c) / 2*a*b); 
                        }
                    }
                }

                for (unsigned int j = 0; j < 3; j++) {
                    float normalsX = triangle_normals[i][0]*weight;
                    float normalsY = triangle_normals[i][1]*weight;
                    float normalsZ = triangle_normals[i][2]*weight;

                    normals[triangles[i][j]] += Vec3(normalsX, normalsY, normalsZ);
                }

            }
            //Iterer sur les normales et les normaliser

            for (unsigned int k = 0; k < normals.size(); k++) {
                normals[k].normalize();
            }
    }

    void computeNormals(int weight_type){
        computeTrianglesNormals();
        computeVerticesNormals(weight_type);
    }


    void displayVoxel(Vec3 centre, double length)  {
        float xmin, xmax, ymin, ymax, zmin, zmax;

        xmin = centre[0]-length/2;
        ymin = centre[1]-length/2;
        zmin = centre[2]-length/2;

        xmax = centre[0]+length/2;
        ymax = centre[1]+length/2;
        zmax = centre[2]+length/2;

        vecMin = Vec3(xmin, ymin, zmin);
        vecMax = Vec3(xmax, ymax, zmax);
    }
    
    float getxMax() {
        return vecMax[0];
    }

    float getyMax() {
        return vecMax[1];
    }

    float getzMax() {
        return vecMax[2];
    }

    float getxMin() {
        return vecMin[0];
    }

};

//Transformation made of a rotation and translation
struct Transformation {
    Mat3 rotation;
    Vec3 translation;
};

//Basis ( origin, i, j ,k )
struct Basis {
    inline Basis ( Vec3 const & i_origin,  Vec3 const & i_i, Vec3 const & i_j, Vec3 const & i_k) {
        origin = i_origin; i = i_i ; j = i_j ; k = i_k;
    }

    inline Basis ( ) {
        origin = Vec3(0., 0., 0.);
        i = Vec3(1., 0., 0.) ; j = Vec3(0., 1., 0.) ; k = Vec3(0., 0., 1.);
    }
    Vec3 operator [] (unsigned int ib) {
        if(ib==0) return i;
        if(ib==1) return j;
        return k;}

    Vec3 origin;
    Vec3 i;
    Vec3 j;
    Vec3 k;
};


//Fonction à completer
void collect_one_ring (std::vector<Vec3> const & i_vertices,
                       std::vector< Triangle > const & i_triangles,
                       std::vector<std::vector<unsigned int> > & o_one_ring) {//one-ring of each vertex, i.e. a list of vertices with which it shares an edge

    //Initialiser le vecteur de o_one_ring de la taille du vecteur vertices
    o_one_ring.clear();
    o_one_ring.resize(i_vertices.size());

    for (long unsigned int i = 0; i < i_vertices.size(); i++) {
        //Parcourir les triangles et ajouter les voisins dans le 1-voisinage
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                //Attention verifier que l'indice n'est pas deja present
                if (std::find(std::begin(o_one_ring[i_triangles[i][j]]), std::end(o_one_ring[i_triangles[i][j]]), i_triangles[i][k]) == std::end(o_one_ring[i_triangles[i][j]]) && i_triangles[i][j] != i_triangles[i][k]) {
                    o_one_ring[i_triangles[i][j]].push_back(i_triangles[i][k]);
                }
            }
        }

        //Tous les points opposés dans le triangle sont reliés
    }

}

//Fonction à completer
void compute_vertex_valences (const std::vector<Vec3> & i_vertices,
                              const std::vector< Triangle > & i_triangles,
                              std::vector<unsigned int> & o_valences ) {
    //Utiliser la fonction collect_one_ring pour récuperer le 1-voisinage
    std::vector<std::vector<unsigned int> > o_one_ring;
    collect_one_ring(i_vertices, i_triangles, o_one_ring);

    for (long unsigned int i = 0; i < o_one_ring.size(); i++) {
        o_valences.push_back(o_one_ring[i].size());
    }
}

// Fonctions que j'ai ajoutées
float min_valence (std::vector<unsigned int> i_valence) {
    unsigned int min = i_valence[0]; 

    for (unsigned int i = 0; i < i_valence.size(); i++) {
        if (i_valence[i]< min) {
            min = i_valence[i];
        }
    }

    return (float)min;
}

float max_valence (std::vector<unsigned int> i_valence) {
    unsigned int max = i_valence[0];

    for (unsigned int i = 0; i < i_valence.size(); i++) {
        if (i_valence[i] > max) {
            max = i_valence[i];
        }
    }

    return (float)max;
}
//

//Input mesh loaded at the launch of the application
Mesh mesh;
std::vector< float > mesh_valence_field; //normalized valence of each vertex
Grid grid;

Basis basis;

bool display_normals;
bool display_smooth_normals;
bool display_mesh;
bool display_basis;
DisplayMode displayMode;
int weight_type;
//
bool display_simplify;

// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 1600;
static unsigned int SCREENHEIGHT = 900;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static bool fullScreen = false;

// ------------------------------------
// File I/O
// ------------------------------------
bool saveOFF( const std::string & filename ,
              std::vector< Vec3 > const & i_vertices ,
              std::vector< Vec3 > const & i_normals ,
              std::vector< Triangle > const & i_triangles,
              std::vector< Vec3 > const & i_triangle_normals ,
              bool save_normals = true ) {
    std::ofstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    myfile << "OFF" << std::endl ;

    unsigned int n_vertices = i_vertices.size() , n_triangles = i_triangles.size();
    myfile << n_vertices << " " << n_triangles << " 0" << std::endl;

    for( unsigned int v = 0 ; v < n_vertices ; ++v ) {
        myfile << i_vertices[v][0] << " " << i_vertices[v][1] << " " << i_vertices[v][2] << " ";
        if (save_normals) myfile << i_normals[v][0] << " " << i_normals[v][1] << " " << i_normals[v][2] << std::endl;
        else myfile << std::endl;
    }
    for( unsigned int f = 0 ; f < n_triangles ; ++f ) {
        myfile << 3 << " " << i_triangles[f][0] << " " << i_triangles[f][1] << " " << i_triangles[f][2]<< " ";
        if (save_normals) myfile << i_triangle_normals[f][0] << " " << i_triangle_normals[f][1] << " " << i_triangle_normals[f][2];
        myfile << std::endl;
    }
    myfile.close();
    return true;
}

void openOFF( std::string const & filename,
              std::vector<Vec3> & o_vertices,
              std::vector<Vec3> & o_normals,
              std::vector< Triangle > & o_triangles,
              std::vector< Vec3 > & o_triangle_normals,
              bool load_normals = true )
{
    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << filename << " cannot be opened" << std::endl;
        return;
    }

    std::string magic_s;

    myfile >> magic_s;

    if( magic_s != "OFF" )
    {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        exit(1);
    }

    int n_vertices , n_faces , dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    o_vertices.clear();
    o_normals.clear();

    for( int v = 0 ; v < n_vertices ; ++v )
    {
        float x , y , z ;

        myfile >> x >> y >> z ;
        o_vertices.push_back( Vec3( x , y , z ) );

        if( load_normals ) {
            myfile >> x >> y >> z;
            o_normals.push_back( Vec3( x , y , z ) );
        }
    }

    o_triangles.clear();
    o_triangle_normals.clear();
    for( int f = 0 ; f < n_faces ; ++f )
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;

        if( n_vertices_on_face == 3 )
        {
            unsigned int _v1 , _v2 , _v3;
            myfile >> _v1 >> _v2 >> _v3;

            o_triangles.push_back(Triangle( _v1, _v2, _v3 ));

            if( load_normals ) {
                float x , y , z ;
                myfile >> x >> y >> z;
                o_triangle_normals.push_back( Vec3( x , y , z ) );
            }
        }
        else if( n_vertices_on_face == 4 )
        {
            unsigned int _v1 , _v2 , _v3 , _v4;
            myfile >> _v1 >> _v2 >> _v3 >> _v4;

            o_triangles.push_back(Triangle(_v1, _v2, _v3 ));
            o_triangles.push_back(Triangle(_v1, _v3, _v4));
            if( load_normals ) {
                float x , y , z ;
                myfile >> x >> y >> z;
                o_triangle_normals.push_back( Vec3( x , y , z ) );
            }

        }
        else
        {
            std::cout << "We handle ONLY *.off files with 3 or 4 vertices per face" << std::endl;
            myfile.close();
            exit(1);
        }
    }

}

// ------------------------------------
// Application initialization
// ------------------------------------
void initLight () {
    GLfloat light_position1[4] = {22.0f, 16.0f, 50.0f, 0.0f};
    GLfloat direction1[3] = {-52.0f,-16.0f,-50.0f};
    GLfloat color1[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

    glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv (GL_LIGHT1, GL_SPECULAR, color1);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
}

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    glCullFace (GL_BACK);
    glDisable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    display_normals = false;
    display_mesh = true;
    display_smooth_normals = true;
    displayMode = LIGHTED;
    display_basis = false;
    //
    display_simplify = false;
}


// ------------------------------------
// Rendering.
// ------------------------------------

void drawVector( Vec3 const & i_from, Vec3 const & i_to ) {

    glBegin(GL_LINES);
    glVertex3f( i_from[0] , i_from[1] , i_from[2] );
    glVertex3f( i_to[0] , i_to[1] , i_to[2] );
    glEnd();
}

void drawAxis( Vec3 const & i_origin, Vec3 const & i_direction ) {

    glLineWidth(4); // for example...
    drawVector(i_origin, i_origin + i_direction);
}

void drawReferenceFrame( Vec3 const & origin, Vec3 const & i, Vec3 const & j, Vec3 const & k ) {

    glDisable(GL_LIGHTING);
    glColor3f( 0.8, 0.2, 0.2 );
    drawAxis( origin, i );
    glColor3f( 0.2, 0.8, 0.2 );
    drawAxis( origin, j );
    glColor3f( 0.2, 0.2, 0.8 );
    drawAxis( origin, k );
    glEnable(GL_LIGHTING);

}

void drawReferenceFrame( Basis & i_basis ) {
    drawReferenceFrame( i_basis.origin, i_basis.i, i_basis.j, i_basis.k );
}

typedef struct {
    float r;       // ∈ [0, 1]
    float g;       // ∈ [0, 1]
    float b;       // ∈ [0, 1]
} RGB;



RGB scalarToRGB( float scalar_value ) //Scalar_value ∈ [0, 1]
{
    RGB rgb;
    float H = scalar_value*360., S = 1., V = 0.85,
            P, Q, T,
            fract;

    (H == 360.)?(H = 0.):(H /= 60.);
    fract = H - floor(H);

    P = V*(1. - S);
    Q = V*(1. - S*fract);
    T = V*(1. - S*(1. - fract));

    if      (0. <= H && H < 1.)
        rgb = (RGB){.r = V, .g = T, .b = P};
    else if (1. <= H && H < 2.)
        rgb = (RGB){.r = Q, .g = V, .b = P};
    else if (2. <= H && H < 3.)
        rgb = (RGB){.r = P, .g = V, .b = T};
    else if (3. <= H && H < 4.)
        rgb = (RGB){.r = P, .g = Q, .b = V};
    else if (4. <= H && H < 5.)
        rgb = (RGB){.r = T, .g = P, .b = V};
    else if (5. <= H && H < 6.)
        rgb = (RGB){.r = V, .g = P, .b = Q};
    else
        rgb = (RGB){.r = 0., .g = 0., .b = 0.};

    return rgb;
}

void drawSmoothTriangleMesh( Mesh const & i_mesh , bool draw_field = false ) {
    glBegin(GL_TRIANGLES);
    for(unsigned int tIt = 0 ; tIt < i_mesh.triangles.size(); ++tIt) {

        for(unsigned int i = 0 ; i < 3 ; i++) {
            const Vec3 & p = i_mesh.vertices[i_mesh.triangles[tIt][i]]; //Vertex position
            const Vec3 & n = i_mesh.normals[i_mesh.triangles[tIt][i]]; //Vertex normal

            if( draw_field && mesh_valence_field.size() > 0 ){
                RGB color = scalarToRGB( mesh_valence_field[i_mesh.triangles[tIt][i]] );
                glColor3f( color.r, color.g, color.b );
            }
            glNormal3f( n[0] , n[1] , n[2] );
            glVertex3f( p[0] , p[1] , p[2] );
        }
    }
    glEnd();

}

void drawTriangleMesh( Mesh const & i_mesh , bool draw_field = false  ) {
    glBegin(GL_TRIANGLES);
    for(unsigned int tIt = 0 ; tIt < i_mesh.triangles.size(); ++tIt) {
        const Vec3 & n = i_mesh.triangle_normals[ tIt ]; //Triangle normal
        for(unsigned int i = 0 ; i < 3 ; i++) {
            const Vec3 & p = i_mesh.vertices[i_mesh.triangles[tIt][i]]; //Vertex position

            if( draw_field ){
                RGB color = scalarToRGB( mesh_valence_field[i_mesh.triangles[tIt][i]] );
                glColor3f( color.r, color.g, color.b );
            }
            glNormal3f( n[0] , n[1] , n[2] );
            glVertex3f( p[0] , p[1] , p[2] );
        }
    }
    glEnd();

}

void drawMesh( Mesh const & i_mesh , bool draw_field = false ){
    if(display_smooth_normals)
        drawSmoothTriangleMesh(i_mesh, draw_field) ; //Smooth display with vertices normals
    else
        drawTriangleMesh(i_mesh, draw_field) ; //Display with face normals
}

void drawVectorField( std::vector<Vec3> const & i_positions, std::vector<Vec3> const & i_directions ) {
    glLineWidth(1.);
    for(unsigned int pIt = 0 ; pIt < i_directions.size() ; ++pIt) {
        Vec3 to = i_positions[pIt] + 0.02*i_directions[pIt];
        drawVector(i_positions[pIt], to);
    }
}

void drawNormals(Mesh const& i_mesh){

    if(display_smooth_normals){
        drawVectorField( i_mesh.vertices, i_mesh.normals );
    } else {
        std::vector<Vec3> triangle_baricenters;
        for ( const Triangle& triangle : i_mesh.triangles ){
            Vec3 triangle_baricenter (0.,0.,0.);
            for( unsigned int i = 0 ; i < 3 ; i++ )
                triangle_baricenter += i_mesh.vertices[triangle[i]];
            triangle_baricenter /= 3.;
            triangle_baricenters.push_back(triangle_baricenter);
        }

        drawVectorField( triangle_baricenters, i_mesh.triangle_normals );
    }
}

//
void drawVoxelGrid() {

    //mesh.calculBoiteEnglobante();
    mesh.displayVoxel(Vec3(0, 0, 0), 1);

    Vec3 Min1, Min2, Min3, Min4, Max1, Max2, Max3, Max4;
        Min1 = Vec3(mesh.vecMin[0], mesh.vecMin[1], mesh.vecMin[2]);
        Min2 = Vec3(mesh.vecMax[0], mesh.vecMin[1], mesh.vecMin[2]);
        Min3 = Vec3(mesh.vecMax[0], mesh.vecMin[1], mesh.vecMax[2]);
        Min4 = Vec3(mesh.vecMin[0], mesh.vecMin[1], mesh.vecMax[2]);

        Max1 = Vec3(mesh.vecMax[0], mesh.vecMax[1], mesh.vecMax[2]);
        Max2 = Vec3(mesh.vecMax[0], mesh.vecMax[1], mesh.vecMin[2]);
        Max3 = Vec3(mesh.vecMin[0], mesh.vecMax[1], mesh.vecMin[2]);
        Max4 = Vec3(mesh.vecMin[0], mesh.vecMax[1], mesh.vecMax[2]);

        drawVector(Min1, Min2);
        drawVector(Min2, Min3);
        drawVector(Min3, Min4);
        drawVector(Min4, Min1);

        drawVector(Min1, Max3);
        drawVector(Min2, Max2);
        drawVector(Min3, Max1);
        drawVector(Min4, Max4);

        drawVector(Max1, Max2);
        drawVector(Max2, Max3);
        drawVector(Max3, Max4);
        drawVector(Max4, Max1);
}

void drawVoxel(float x, float y, float z, float sizeVoxel) {
    mesh.displayVoxel(Vec3(x, y, z), sizeVoxel);

    Vec3 Min1, Min2, Min3, Min4, Max1, Max2, Max3, Max4;
        Min1 = Vec3(mesh.vecMin[0], mesh.vecMin[1], mesh.vecMin[2]);
        Min2 = Vec3(mesh.vecMax[0], mesh.vecMin[1], mesh.vecMin[2]);
        Min3 = Vec3(mesh.vecMax[0], mesh.vecMin[1], mesh.vecMax[2]);
        Min4 = Vec3(mesh.vecMin[0], mesh.vecMin[1], mesh.vecMax[2]);

        Max1 = Vec3(mesh.vecMax[0], mesh.vecMax[1], mesh.vecMax[2]);
        Max2 = Vec3(mesh.vecMax[0], mesh.vecMax[1], mesh.vecMin[2]);
        Max3 = Vec3(mesh.vecMin[0], mesh.vecMax[1], mesh.vecMin[2]);
        Max4 = Vec3(mesh.vecMin[0], mesh.vecMax[1], mesh.vecMax[2]);

        glBegin(GL_QUADS);
        glVertex3f(Min4[0], Min4[1], Min4[2]);
        glVertex3f(Min3[0], Min3[1], Min3[2]);
        glVertex3f(Min2[0], Min2[1], Min2[2]);
        glVertex3f(Min1[0], Min1[1], Min1[2]);
        
        glVertex3f(Max1[0], Max1[1], Max1[2]);
        glVertex3f(Max2[0], Max2[1], Max2[2]);
        glVertex3f(Max3[0], Max3[1], Max3[2]);
        glVertex3f(Max4[0], Max4[1], Max4[2]);
      
        glVertex3f(Min2[0], Min2[1], Min2[2]);
        glVertex3f(Min3[0], Min3[1], Min3[2]);
        glVertex3f(Max1[0], Max1[1], Max1[2]);
        glVertex3f(Max2[0], Max2[1], Max2[2]);
     
        glVertex3f(Min3[0], Min3[1], Min3[2]);
        glVertex3f(Min4[0], Min4[1], Min4[2]);
        glVertex3f(Max4[0], Max4[1], Max4[2]);
        glVertex3f(Max1[0], Max1[1], Max1[2]);
 
        glVertex3f(Min4[0], Min4[1], Min4[2]);
        glVertex3f(Min1[0], Min1[1], Min1[2]);
        glVertex3f(Max3[0], Max3[1], Max3[2]);
        glVertex3f(Max4[0], Max4[1], Max4[2]);
   
        glVertex3f(Min1[0], Min1[1], Min1[2]);
        glVertex3f(Min2[0], Min2[1], Min2[2]);
        glVertex3f(Max2[0], Max2[1], Max2[2]);
        glVertex3f(Max3[0], Max3[1], Max3[2]);
        glEnd();
}

bool isInASphere(Vec3 point, Vec3 centre, double rayon) {
    if (pow(point[0]-centre[0], 2) + pow(point[1]-centre[1], 2) + pow(point[2]-centre[2], 2) < pow(rayon, 2)) {
        return true;
    }

    return false;
}

void displaySphereVolumic(Vec3 centre, double rayon, double resolution) {
    float sizeVox = 1;
    for (int i = 1; i < resolution; i++) {
        sizeVox = (float)sizeVox/2;
    }

    int nbPositions = pow(2, (resolution-1));

    int nPosX = 0;
    int nPosY = 0;
    int nPosZ = 0;
    float posVoxX;
    float posVoxY;
    float posVoxZ;

    if(resolution == 1) {
        if (isInASphere(Vec3(0, 0, 0), centre, rayon)) {
            drawVoxel(0, 0, 0, sizeVox);
        }
    }
    else {
        for (float i = 0; i < 0.5; i += sizeVox) {
            posVoxX = sizeVox/2 + nPosX*sizeVox;
            nPosY = 0;

            for (float j = 0; j < 0.5; j+=sizeVox) {
                posVoxY = sizeVox/2 + nPosY*sizeVox;
                nPosZ = 0;

                for (float k = 0; k < 0.5; k+=sizeVox) {
                    posVoxZ = sizeVox/2 + nPosZ*sizeVox;

                    if (isInASphere(Vec3(posVoxX, posVoxY, posVoxZ), centre, rayon)) {
                        drawVoxel(posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, -posVoxZ, sizeVox);
                    }
                    nPosZ++;
                }
                nPosY++;
            }
            nPosX++;
        }
    }

}

bool isInACylinder(Vec3 point, Vec3 axisVector, Vec3 axisOrigin, float rayon) {
    Vec3 axisUp = axisOrigin + axisVector;
    float c = rayon * axisVector.length();

    float conditionToCheck1 = Vec3::dot(point - axisOrigin, axisVector);
    float conditionToCheck2 = Vec3::dot(point - axisUp, axisVector);
    float conditionToCheck3 = (Vec3::cross(point - axisOrigin, axisVector)).length();

    if (conditionToCheck1 >= 0 && conditionToCheck2 <= 0 && conditionToCheck3 <= rayon) {
        return true;
    }
    return false;

}

void displayCylinderVolumic(Vec3 axisOrigin, Vec3 axisVector, double rayon, double resolution) {
    float sizeVox = 1;
    for (int i = 1; i < resolution; i++) {
        sizeVox = (float)sizeVox/2;
    }

    int nbPositions = pow(2, (resolution-1));

    int nPosX = 0;
    int nPosY = 0;
    int nPosZ = 0;
    float posVoxX;
    float posVoxY;
    float posVoxZ;

    if(resolution == 1) {
        if (isInACylinder(Vec3(0, 0, 0), axisVector, axisOrigin, rayon)) {
            drawVoxel(0, 0, 0, sizeVox);
        }
    }
    else {
        for (float i = 0; i < 0.5; i += sizeVox) {
            posVoxX = sizeVox/2 + nPosX*sizeVox;
            nPosY = 0;

            for (float j = 0; j < 0.5; j+=sizeVox) {
                posVoxY = sizeVox/2 + nPosY*sizeVox;
                nPosZ = 0;

                for (float k = 0; k < 0.5; k+=sizeVox) {
                    posVoxZ = sizeVox/2 + nPosZ*sizeVox;

                    if (isInACylinder(Vec3(posVoxX, posVoxY, posVoxZ), axisVector, axisOrigin, rayon)) {
                        drawVoxel(posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, -posVoxZ, sizeVox);
                    }
                    nPosZ++;
                }
                nPosY++;
            }
            nPosX++;
        }
    }
}

void display_INTERSECTION_SphereCylinder(Vec3 centreSphere, double rayonSphere, Vec3 axisOriginCylinder, Vec3 axisVectorCylinder, double rayonCylinder, double resolution) {
    float sizeVox = 1;
    for (int i = 1; i < resolution; i++) {
        sizeVox = (float)sizeVox/2;
    }

    int nbPositions = pow(2, (resolution-1));

    int nPosX = 0;
    int nPosY = 0;
    int nPosZ = 0;
    float posVoxX;
    float posVoxY;
    float posVoxZ;

    if(resolution == 1) {
        if (isInACylinder(Vec3(0, 0, 0), axisVectorCylinder, axisOriginCylinder, rayonCylinder) && isInASphere(Vec3(0, 0, 0), centreSphere, rayonSphere)) {
            drawVoxel(0, 0, 0, sizeVox);
        }
    }
    else {
        for (float i = 0; i < 0.5; i += sizeVox) {
            posVoxX = sizeVox/2 + nPosX*sizeVox;
            nPosY = 0;

            for (float j = 0; j < 0.5; j+=sizeVox) {
                posVoxY = sizeVox/2 + nPosY*sizeVox;
                nPosZ = 0;

                for (float k = 0; k < 0.5; k+=sizeVox) {
                    posVoxZ = sizeVox/2 + nPosZ*sizeVox;

                    if (isInACylinder(Vec3(posVoxX, posVoxY, posVoxZ), axisVectorCylinder, axisOriginCylinder, rayonCylinder) && isInASphere(Vec3(posVoxX, posVoxY, posVoxZ), centreSphere, rayonSphere)) {
                        drawVoxel(posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, -posVoxZ, sizeVox);
                    }
                    nPosZ++;
                }
                nPosY++;
            }
            nPosX++;
        }
    }
}

void display_SOUSTRACTION_SphereCylinder(Vec3 centreSphere, double rayonSphere, Vec3 axisOriginCylinder, Vec3 axisVectorCylinder, double rayonCylinder, double resolution) {
    float sizeVox = 1;
    for (int i = 1; i < resolution; i++) {
        sizeVox = (float)sizeVox/2;
    }

    int nbPositions = pow(2, (resolution-1));

    int nPosX = 0;
    int nPosY = 0;
    int nPosZ = 0;
    float posVoxX;
    float posVoxY;
    float posVoxZ;

    if(resolution == 1) {
        if (isInASphere(Vec3(0, 0, 0), centreSphere, rayonSphere) && !isInACylinder(Vec3(0, 0, 0), axisVectorCylinder, axisOriginCylinder, rayonCylinder)) {
            drawVoxel(0, 0, 0, sizeVox);
        }
    }
    else {
        for (float i = 0; i < 0.5; i += sizeVox) {
            posVoxX = sizeVox/2 + nPosX*sizeVox;
            nPosY = 0;

            for (float j = 0; j < 0.5; j+=sizeVox) {
                posVoxY = sizeVox/2 + nPosY*sizeVox;
                nPosZ = 0;

                for (float k = 0; k < 0.5; k+=sizeVox) {
                    posVoxZ = sizeVox/2 + nPosZ*sizeVox;

                    if (isInASphere(Vec3(posVoxX, posVoxY, posVoxZ), centreSphere, rayonSphere) && !isInACylinder(Vec3(posVoxX, posVoxY, posVoxZ), axisVectorCylinder, axisOriginCylinder, rayonCylinder)) {
                        drawVoxel(posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, -posVoxZ, sizeVox);
                    }
                    nPosZ++;
                }
                nPosY++;
            }
            nPosX++;
        }
    }
}

void display_UNION_SphereCylinder(Vec3 centreSphere, double rayonSphere, Vec3 axisOriginCylinder, Vec3 axisVectorCylinder, double rayonCylinder, double resolution) {
    float sizeVox = 1;
    for (int i = 1; i < resolution; i++) {
        sizeVox = (float)sizeVox/2;
    }

    int nbPositions = pow(2, (resolution-1));

    int nPosX = 0;
    int nPosY = 0;
    int nPosZ = 0;
    float posVoxX;
    float posVoxY;
    float posVoxZ;

    if(resolution == 1) {
        if (isInASphere(Vec3(0, 0, 0), centreSphere, rayonSphere) || isInACylinder(Vec3(0, 0, 0), axisVectorCylinder, axisOriginCylinder, rayonCylinder)) {
            drawVoxel(0, 0, 0, sizeVox);
        }
    }
    else {
        for (float i = 0; i < 0.5; i += sizeVox) {
            posVoxX = sizeVox/2 + nPosX*sizeVox;
            nPosY = 0;

            for (float j = 0; j < 0.5; j+=sizeVox) {
                posVoxY = sizeVox/2 + nPosY*sizeVox;
                nPosZ = 0;

                for (float k = 0; k < 0.5; k+=sizeVox) {
                    posVoxZ = sizeVox/2 + nPosZ*sizeVox;

                    if (isInASphere(Vec3(posVoxX, posVoxY, posVoxZ), centreSphere, rayonSphere) || isInACylinder(Vec3(posVoxX, posVoxY, posVoxZ), axisVectorCylinder, axisOriginCylinder, rayonCylinder)) {
                        drawVoxel(posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, posVoxZ, sizeVox);
                        drawVoxel(posVoxX, posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(posVoxX, -posVoxY, -posVoxZ, sizeVox);
                        drawVoxel(-posVoxX, posVoxY, -posVoxZ, sizeVox);
                    }
                    nPosZ++;
                }
                nPosY++;
            }
            nPosX++;
        }
    }
}

//Draw fonction
void draw () {

    if(displayMode == LIGHTED || displayMode == LIGHTED_WIRE){

        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);

    }  else if(displayMode == WIRE){

        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        glDisable (GL_LIGHTING);

    }  else if(displayMode == SOLID ){
        glDisable (GL_LIGHTING);
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

    }

    glColor3f(0.8,1,0.8);
    drawMesh(mesh, true);

    drawVoxelGrid();
    //Ex1
    //drawVoxel(0, 0, 0, 1);
    //drawVoxel(-0.25, 0, 0, 0.25);

    //Ex2
    //displaySphereVolumic(Vec3(0, 0, 0), 0.25, 6);
    //displaySphereVolumic(Vec3(0, 0, 0), 0.5, 3);
    //displayCylinderVolumic(Vec3(0, 0, 0), Vec3(1, 0, 0), 0.25, 5);
    //displayCylinderVolumic(Vec3(0, 0, 0), Vec3(0, 1, 0), 0.5, 8);

    //Ex3
    //display_INTERSECTION_SphereCylinder(Vec3(0, 0, 0), 0.5, Vec3(0, 0, 0), Vec3(0, 1, 0), 0.25, 6);
    //display_INTERSECTION_SphereCylinder(Vec3(0, 0, 0), 0.5, Vec3(0, 0, 0), Vec3(0, 1, 0), 0.25, 8);
    //display_SOUSTRACTION_SphereCylinder(Vec3(0, 0, 0), 0.5, Vec3(0, 0, 0), Vec3(0, 1, 0), 0.25, 6);
    display_UNION_SphereCylinder(Vec3(0, 0, 0), 0.5, Vec3(0, 0, 0), Vec3(0, 1, 0), 0.25, 6);

    if(displayMode == SOLID || displayMode == LIGHTED_WIRE){
        glEnable (GL_POLYGON_OFFSET_LINE);
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth (1.0f);
        glPolygonOffset (-2.0, 1.0);

        glColor3f(0.,0.,0.);
        drawMesh(mesh, false);

        glDisable (GL_POLYGON_OFFSET_LINE);
        glEnable (GL_LIGHTING);
    }



    glDisable(GL_LIGHTING);
    if(display_normals){
        glColor3f(1.,0.,0.);
        drawNormals(mesh);
    }

    if( display_basis ){
        drawReferenceFrame(basis);
    }
    glEnable(GL_LIGHTING);

}

void changeDisplayMode(){
    if(displayMode == LIGHTED)
        displayMode = LIGHTED_WIRE;
    else if(displayMode == LIGHTED_WIRE)
        displayMode = SOLID;
    else if(displayMode == SOLID)
        displayMode = WIRE;
    else
        displayMode = LIGHTED;
}

void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw ();
    glFlush ();
    glutSwapBuffers ();
}

void idle () {
    glutPostRedisplay ();
}

// ------------------------------------
// User inputs
// ------------------------------------
//Keyboard event
void key (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }
        break;


    case 'w': //Change le mode d'affichage
        changeDisplayMode();
        break;


    case 'b': //Toggle basis display
        display_basis = !display_basis;
        break;

    case 'n': //Press n key to display normals
        display_normals = !display_normals;
        break;

    case '1': //Toggle loaded mesh display
        display_mesh = !display_mesh;
        break;

    case 's': //Switches between face normals and vertices normals
        display_smooth_normals = !display_smooth_normals;
        break;

    /*case 'a' : 
        if (!display_simplify) {
            display_simplify = true;
            grid.resolution = 64;
            mesh.simplify(grid);
            mesh.computeNormals(weight_type);
        }
        break;*/

    case '+': //Changes weight type: 0 uniforme, 1 aire des triangles, 2 angle du triangle
        weight_type ++;
        if(weight_type == 3) weight_type = 0;
        mesh.computeVerticesNormals(weight_type); //recalcul des normales avec le type de poids choisi
        break;

    default:
        break;
    }
    idle ();
}

//Mouse events
void mouse (int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }

    idle ();
}

//Mouse motion, update camera
void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
}

// ------------------------------------
// Start of graphical application
// ------------------------------------
int main (int argc, char ** argv) {
    if (argc > 2) {
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("TP HAI702I");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);

    //Mesh loaded with precomputed normals
   // openOFF("data/sphere.off", mesh.vertices, mesh.normals, mesh.triangles, mesh.triangle_normals);

    //Completer les fonction de calcul de normals
    mesh.computeNormals(weight_type);

    basis = Basis();

    // A faire : completer la fonction compute_vertex_valences pour calculer les valences
    //***********************************************//
   // std::vector<unsigned int> valences;
    // TODO : Question 1 le calcul des valence pour chaques sommets (vertices) remplir le vector valences
    //          Le nombre de sommets voisin au sommet donné ( partage une arête )
    //          TODO : collect_one_ring() [ Permet de collecter le 1-voisinage ]

    //compute_vertex_valences(mesh.vertices, mesh.triangles, valences);

    // A faire : normaliser les valences pour avoir une valeur flotante entre 0. et 1. dans mesh_valence_field
    //***********************************************//
    // Utile pour la question 2 permettant d'afficher une couleur dépendant de la valence des sommets
    /*
    mesh_valence_field.clear();

    float minvalence = min_valence(valences);
    float maxvalence = max_valence(valences);

    mesh_valence_field.resize(valences.size());

    for (long unsigned int i = 0; i < valences.size(); i++) {
        mesh_valence_field[i] = (valences[i]-minvalence)/(maxvalence-minvalence);
    }*/

    glutMainLoop ();
    return EXIT_SUCCESS;
}

