// Objects.cpp
// OpenGL SuperBible, Chapter 4
// Demonstrates GLTools built-in objects
// Program by Richard S. Wright Jr.

/*
 1、矩阵堆栈的使用
 2、GLFrame的使用
 3、GLFrustum视景体的使用，用来构造投影矩阵
 4、渲染技巧的复习
 
 关注核心过程：
 1、投影视图、物体视图进行变换
 2、得到视图的变换矩阵与模型视图矩阵计算，就可以变换图形
 */

#include "GLTools.h"	// OpenGL toolkit
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

//固定管道
GLShaderManager		shaderManager;
//矩阵堆栈
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
//观察者位置
GLFrame				cameraFrame;
//世界坐标位置
GLFrame             objectFrame;

//视景体，用来构造投影矩阵
GLFrustum			viewFrustum;

//三角形批次类
GLTriangleBatch     CC_Triangle;

//球
GLTriangleBatch     sphereBatch;
//环
GLTriangleBatch     torusBatch;
//圆柱
GLTriangleBatch     cylinderBatch;
//锥
GLTriangleBatch     coneBatch;
//磁盘
GLTriangleBatch     diskBatch;

GLGeometryTransform	transformPipeline;
M3DMatrix44f		shadowMatrix;

GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

int nStep = 0;

// 将上下文中，进行必要的初始化
void SetupRC()
{
    //1.
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f );
    shaderManager.InitializeStockShaders();
    
    //2.开启深度测试
    glEnable(GL_DEPTH_TEST);

    //3.将观察者坐标位置Z移动往屏幕里移动15个单位位置
    //表示离屏幕之间的距离 负数，是往屏幕后面移动；正数，往屏幕前面移动
    /*
     设置观察者坐标到物体的距离
     物体坐标的Z轴正方向指向屏幕外
     因此观察者想要离物体更近，需要负的15个单位
     */
    cameraFrame.MoveForward(-15.0f);
    
    /*
     如果使用物体坐标，则刚好相反，需要用正的15个单位
     */
//    objectFrame.MoveForward(15.0f);

    //4.利用三角形批次类构造图形对象
    // 球
    /*
      gltMakeSphere(GLTriangleBatch& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks);
     参数1：sphereBatch，三角形批次类对象
     参数2：fRadius，球体半径
     参数3：iSlices，从球体底部堆叠到顶部的三角形带的数量；其实球体是一圈一圈三角形带组成
     参数4：iStacks，围绕球体一圈排列的三角形对数
     
     建议：一个对称性较好的球体的片段数量是堆叠数量的2倍，就是iStacks = 2 * iSlices;
     绘制球体都是围绕Z轴，这样+z就是球体的顶点，-z就是球体的底部。
     */
    gltMakeSphere(sphereBatch, 3.0, 10, 20);
    
    // 环面
    /*
     gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
     参数1：torusBatch，三角形批次类对象
     参数2：majorRadius,甜甜圈中心到外边缘的半径
     参数3：minorRadius,甜甜圈中心到内边缘的半径
     参数4：numMajor,沿着主半径的三角形数量
     参数5：numMinor,沿着内部较小半径的三角形数量
     */
    gltMakeTorus(torusBatch, 3.0f, 0.75f, 15, 15);
    
    // 圆柱
    /*
     void gltMakeCylinder(GLTriangleBatch& cylinderBatch, GLfloat baseRadius, GLfloat topRadius, GLfloat fLength, GLint numSlices, GLint numStacks);
     参数1：cylinderBatch，三角形批次类对象
     参数2：baseRadius,底部半径
     参数3：topRadius,头部半径
     参数4：fLength,圆形长度
     参数5：numSlices,围绕Z轴的三角形对的数量
     参数6：numStacks,圆柱底部堆叠到顶部圆环的三角形数量
     */
    gltMakeCylinder(cylinderBatch, 2.0f, 2.0f, 3.0f, 15, 2);
    
    //锥
    /*
     void gltMakeCylinder(GLTriangleBatch& cylinderBatch, GLfloat baseRadius, GLfloat topRadius, GLfloat fLength, GLint numSlices, GLint numStacks);
     参数1：cylinderBatch，三角形批次类对象
     参数2：baseRadius,底部半径
     参数3：topRadius,头部半径
     参数4：fLength,圆形长度
     参数5：numSlices,围绕Z轴的三角形对的数量
     参数6：numStacks,圆柱底部堆叠到顶部圆环的三角形数量
     */
    //圆柱体，从0开始向Z轴正方向延伸。
    //圆锥体，是一端的半径为0，另一端半径可指定。
    gltMakeCylinder(coneBatch, 2.0f, 0.0f, 3.0f, 13, 2);
    
    // 磁盘
    /*
    void gltMakeDisk(GLTriangleBatch& diskBatch, GLfloat innerRadius, GLfloat outerRadius, GLint nSlices, GLint nStacks);
     参数1:diskBatch，三角形批次类对象
     参数2:innerRadius,内圆半径
     参数3:outerRadius,外圆半径
     参数4:nSlices,圆盘围绕Z轴的三角形对的数量
     参数5:nStacks,圆盘外网到内围的三角形数量
     */
    gltMakeDisk(diskBatch, 1.5f, 3.0f, 13, 3);
}



void DrawWireFramedBatch(GLTriangleBatch* pBatch)
{
    //----绘制图形----
    //1.平面着色器，绘制三角形
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
     //传过来的参数，对应不同的图形Batch
    pBatch->Draw();
    
    //---画出黑色轮廓---
    
    //2.开启多边形偏移
    glEnable(GL_POLYGON_OFFSET_LINE);
    //多边形模型(背面、线) 将多边形背面设为线框模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //开启多边形偏移(设置偏移数量)
    glPolygonOffset(-1.0f, -1.0f);
    //线条宽度
    glLineWidth(2.5f);
    
    //3.开启混合功能(颜色混合&抗锯齿功能)
    glEnable(GL_BLEND);
    //开启处理线段抗锯齿功能
    glEnable(GL_LINE_SMOOTH);
    //设置颜色混合因子
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   
   //4.平面着色器绘制线条
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    
    pBatch->Draw();
    
    //5.恢复多边形模式和深度测试
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
}

/*
 1、这里没有涉及到投影，只涉及模型视图矩阵
 2、模型视图矩阵前面只加载了单元矩阵
 3、模型视图矩阵与摄像机矩阵、物体矩阵相乘，可以进行图形变换
 */

//召唤场景
void RenderScene(void)
{
    //1.用当前清除颜色清除窗口背景
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    //2.模型视图矩阵栈堆，压栈
    modelViewMatrix.PushMatrix();//copy一份栈顶指针，并压栈
    
    
    //3.获取摄像头矩阵
    M3DMatrix44f mCamera;
    //从camereaFrame中获取矩阵，赋给mCamera
    cameraFrame.GetCameraMatrix(mCamera);
    //模型视图堆栈的 矩阵与mCamera矩阵 相乘之后，存储到modelViewMatrix矩阵堆栈中
    //也就是刚才的栈顶矩阵现在已经变成了MVP矩阵了
    modelViewMatrix.MultMatrix(mCamera);
    
    //4.创建矩阵mObjectFrame
    M3DMatrix44f mObjectFrame;
    //从ObjectFrame 获取矩阵到mOjectFrame中
    objectFrame.GetMatrix(mObjectFrame);
    //将modelViewMatrix 的堆栈中的矩阵 与 mOjbectFrame 矩阵相乘，存储到modelViewMatrix矩阵堆栈中
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    /*
     因为这里观察者和物体都移动，观察者向前移动，物体旋转，所以两者需要压栈相乘
     如果只移动一个，则直接压栈即可。
     modelViewMatrix.PushMatrix(cameraFrame);
     在进行压栈的时候，会先复制一份栈顶指针并与矩阵相乘，之后压栈
     而且内部会帮我们做一个objectFrame.GetMatrix(mObjectFrame);的操作
     
     看源代码，不能放入cameraFrame。因为内部没有调用GetCameraMatrix
     */
    
    //模型视图矩阵乘以观察者矩阵，再乘以坐标位置，顺序不能倒。
    //顺序是
    
    //5.判断你目前是绘制第几个图形
    switch(nStep) {
        case 0:
            DrawWireFramedBatch(&sphereBatch);
            break;
        case 1:
            DrawWireFramedBatch(&torusBatch);
            break;
        case 2:
            DrawWireFramedBatch(&cylinderBatch);
            break;
        case 3:
            DrawWireFramedBatch(&coneBatch);
            break;
        case 4:
            DrawWireFramedBatch(&diskBatch);
            break;
    }
    
    //6. pop
    modelViewMatrix.PopMatrix();
    
    //7.
    glutSwapBuffers();
}




//上下左右，移动图形
void SpecialKeys(int key, int x, int y)
{
    if(key == GLUT_KEY_UP)
        //移动世界坐标系，而不是去移动物体。
        //将世界坐标系在X方向移动-5.0
    objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_DOWN)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_LEFT)
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    
    glutPostRedisplay();
}




//点击空格，切换渲染图形
void KeyPressFunc(unsigned char key, int x, int y)
{
    if(key == 32)
    {
        nStep++;
        
        if(nStep > 4)
            nStep = 0;
    }
    
    switch(nStep)
    {
        case 0:
            glutSetWindowTitle("Sphere");
            break;
        case 1:
            glutSetWindowTitle("Torus");
            break;
        case 2:
            glutSetWindowTitle("Cylinder");
            break;
        case 3:
            glutSetWindowTitle("Cone");
            break;
        case 4:
            glutSetWindowTitle("Disk");
            break;
    }
    
    glutPostRedisplay();
}


void ChangeSize(int w, int h)
{
    //1.视口
    glViewport(0, 0, w, h);
    
    //2.透视投影
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
    //projectionMatrix 矩阵堆栈 加载透视投影矩阵
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    //3.modelViewMatrix 矩阵堆栈 加载单元矩阵
     modelViewMatrix.LoadIdentity();
    
    //4.通过GLGeometryTransform管理矩阵堆栈
    //使用transformPipeline 管道管理模型视图矩阵堆栈 和 投影矩阵堆栈
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}


int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Sphere");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyPressFunc);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    
    SetupRC();
    
    glutMainLoop();
    return 0;
}
