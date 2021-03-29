/**
 给一个金字塔设置纹理
 1、金字塔的6个三角形使用同一个纹理
 2、使用法线显示阴影效果
 3、金字塔的样式可以查看pdf中的金字塔
 
 学习：
 1、纹理的设置过程
    1）分配纹理对象
    2）绑定纹理
    3）加载纹理文件
        （1）读取纹理
        （2）设置纹理参数
        （3）载入纹理
        （4）设置mip切图
    4）绘制金字塔
        （1）设置金字塔顶点数
        （2）设置金字塔法线
        （3）设置金字塔纹理坐标
 
 2、纹理坐标的理解
 */


#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLFrame.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLShaderManager		shaderManager;
GLMatrixStack		modelViewMatrix;
GLMatrixStack		projectionMatrix;
GLFrame				cameraFrame;
GLFrame             objectFrame;
GLFrustum			viewFrustum;

//金字塔批次类
GLBatch             pyramidBatch;

//纹理变量，一般使用无符号整型
GLuint              textureID;

GLGeometryTransform	transformPipeline;

M3DMatrix44f		shadowMatrix;

//绘制金字塔
/*
 1、创建金字塔批次
 2、设置金字塔顶点数
 3、设置金字塔法线(这里的法线是为了显示光照效果，与纹理没关系）
 4、设置金字塔纹理坐标
 
 主要看如何设置发现和纹理坐标
 */
void MakePyramid(GLBatch& pyramidBatch)
{
    /*1、通过pyramidBatch组建三角形批次
      参数1：类型
      参数2：顶点数
      参数3：这个批次中将会应用1个纹理
      注意：如果不写这个参数，默认为0。
     */
    pyramidBatch.Begin(GL_TRIANGLES, 18, 1);
    
    /*
     * 1、金字塔底部
     */
    //底部的四边形 = 三角形X + 三角形Y
    
    /*
     1、设置法线
     Normal3f：添加一个表面法线（法线坐标 与 Vertex顶点坐标中的Y轴一致）
     表面法线是有方向的向量，代表表面或者顶点面对的方向（相反的方向）。在多数的光照模式下是必须使用。后面的课程会详细来讲法线的应用
     2、设置纹理坐标
      MultiTexCoord2f(GLuint texture,GLclampf s,GLclamp t);
      参数1：texture，纹理层次，对于使用存储着色器来进行渲染，设置为0
      参数2：(s,t,r,q对应顶点坐标的x,y,z,w)s：对应顶点坐标中的x坐标
      参数3：t:对应顶点坐标中的y
     //textrure这里设置为0表示绑定纹理的数组中的第一个纹理。而我们这里只用到了一个纹理。所以就都用0
     //在数组中第一个纹理就是0，所以这里就是0
     //如果有多张图片进行设置，可以看下是否是最下面的那种为0，最上面的数值最大
     */
    //三角形X
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);//这里发现就是与Y轴一致，所以就直接这么写了
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);//纹理的坐标就是一个平面的坐标，这里的左上角就是(0,0)
    //vBlackLeft点
    pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);
   
    
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    //vBlackRight点
    pyramidBatch.Vertex3f(1.0f, -1.0f, -1.0f);
    
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    //vFrontRight点
    pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);
    
    
    //三角形Y
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
    pyramidBatch.Vertex3f(-1.0f, -1.0f, 1.0f);
    
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);
    
    pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
    pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);
    
    //塔顶
    M3DVector3f vApex = { 0.0f, 1.0f, 0.0f };
    M3DVector3f vFrontLeft = { -1.0f, -1.0f, 1.0f };
    M3DVector3f vFrontRight = { 1.0f, -1.0f, 1.0f };
    M3DVector3f vBackLeft = { -1.0f, -1.0f, -1.0f };
    M3DVector3f vBackRight = { 1.0f, -1.0f, -1.0f };
    M3DVector3f n;//法线
    
    
    // 金字塔前面
    //三角形：（Apex，vFrontLeft，vFrontRight）
    //纹理坐标设置，参考PPT图6-4图
    /** 获取从三点找到一个法线坐标(三点确定一个面)
     void m3dFindNormal(result,point1, point2,point3);
     参数1：结果
     参数2-4：3个顶点数据
     */
    m3dFindNormal(n, vApex, vFrontLeft, vFrontRight);
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontLeft);

    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontRight);
    
    //金字塔左边
    //三角形：（vApex, vBackLeft, vFrontLeft）
    m3dFindNormal(n, vApex, vBackLeft, vFrontLeft);
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackLeft);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontLeft);
    
    //金字塔右边
    //三角形：（vApex, vFrontRight, vBackRight）
    m3dFindNormal(n, vApex, vFrontRight, vBackRight);
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vFrontRight);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackRight);
    
    //金字塔后边
    //三角形：（vApex, vBackRight, vBackLeft）
    m3dFindNormal(n, vApex, vBackRight, vBackLeft);
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
    pyramidBatch.Vertex3fv(vApex);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackRight);
    
    pyramidBatch.Normal3fv(n);
    pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
    pyramidBatch.Vertex3fv(vBackLeft);
    
    //结束批次设置
    pyramidBatch.End();
}

// 将TGA文件加载为2D纹理。
/*
 1、打开tag文件读取纹理位和像素信息
 2、设置纹理参数
 3、加载纹理
 4、设置mip贴图
 */
bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
    //定义变量
    GLbyte *pBits;//纹理数据
    int nWidth, nHeight, nComponents;
    GLenum eFormat;//像素格式
    
    //1、解析纹理文件读取到纹理数据、宽高像素、组件、格式
    //参数1：纹理文件名称
    //参数2：文件宽度地址
    //参数3：文件高度地址
    //参数4：文件组件地址
    //参数5：文件格式地址
    //返回值：pBits,指向图像数据的指针
    
    pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
    if(pBits == NULL)
        return false;
    
    //2、设置纹理参数
    //参数1：纹理维度
    //参数2：为S/T坐标设置模式
    //参数3：wrapMode,环绕模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    
    //参数1：纹理维度
    //参数2：线性过滤
    //参数3：wrapMode,环绕模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    
//    //3、精密包装像素数据----一般不用
//    //参数1：GL_UNPACK_ALIGNMENT,指定OpenGL如何从数据缓存区中解包图像数据
//    //参数2:针对GL_UNPACK_ALIGNMENT 设置的值
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    /*
     3、载入纹理
     */
    //参数1：纹理维度
    //参数2：mip贴图层次
    //参数3：纹理单元存储的颜色成分（从读取像素图时获得）
    //参数4：加载纹理宽
    //参数5：加载纹理高
    //参数6：加载纹理的深度
    //参数7：像素数据的数据类型（GL_UNSIGNED_BYTE，每个颜色分量都是一个8位无符号整数）
    //参数8：指向纹理图像数据的指针
    
    glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
                 eFormat, GL_UNSIGNED_BYTE, pBits);
    
    //使用完毕释放pBits
    free(pBits);
    
    /*
     4、设置mip贴图
     */
    //只有minFilter 等于以下四种模式，才可以生成Mip贴图
    //GL_NEAREST_MIPMAP_NEAREST具有非常好的性能，并且闪烁现象非常弱
    //GL_LINEAR_MIPMAP_NEAREST常常用于对游戏进行加速，它使用了高质量的线性过滤器
    //GL_LINEAR_MIPMAP_LINEAR 和GL_NEAREST_MIPMAP_LINEAR 过滤器在Mip层之间执行了一些额外的插值，以消除他们之间的过滤痕迹。
    //GL_LINEAR_MIPMAP_LINEAR 三线性Mip贴图。纹理过滤的黄金准则，具有最高的精度。
    if(minFilter == GL_LINEAR_MIPMAP_LINEAR ||
       minFilter == GL_LINEAR_MIPMAP_NEAREST ||
       minFilter == GL_NEAREST_MIPMAP_LINEAR ||
       minFilter == GL_NEAREST_MIPMAP_NEAREST)
        
        //加载Mip,纹理生成所有的Mip层
        //参数：GL_TEXTURE_1D、GL_TEXTURE_2D、GL_TEXTURE_3D
        glGenerateMipmap(GL_TEXTURE_2D);
    
    return true;
}


/*
 在初始化的时候加载纹理
 
 这三个纹理设置方法的顺序可以打乱，没有固定顺序
 
 1、分配纹理对象
 2、绑定纹理状态
 3、加载纹理
 */

void SetupRC()
{
    //设置颜色
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f );
    
    //初始化固定管线
    shaderManager.InitializeStockShaders();
    
    //开启深度测试
    glEnable(GL_DEPTH_TEST);
    
    //分配纹理对象 参数1:纹理对象个数，参数2:纹理对象指针
    glGenTextures(1, &textureID);
    //绑定纹理状态 参数1：纹理状态2D 参数2：纹理对象
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    /*
     *私有方法，用来加载纹理
     *将TGA文件加载为2D纹理。
     *参数1：纹理文件名称
     *参数2&参数3：需要缩小&放大的过滤器
     *参数4：纹理坐标环绕模式
     */
    LoadTGATexture("stone.tga", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_CLAMP_TO_EDGE);
    
    //创造金字塔pyramidBatch
    MakePyramid(pyramidBatch);
    
    /**相机frame MoveForward(平移)
    参数1：Z，深度（屏幕到图形的Z轴距离）
     */
    cameraFrame.MoveForward(-10);
}



// 清理…例如删除纹理对象，系统调用，类似于alloc
void ShutdownRC(void)
{
    //参数
    glDeleteTextures(1, &textureID);
}



void RenderScene(void)
{
    //颜色值
    static GLfloat vLightPos [] = { 1.0f, 1.0f, 0.0f };
    static GLfloat vWhite [] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    //清理缓存区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    //将当前栈顶的模型视图矩阵复制一份并压栈
    modelViewMatrix.PushMatrix();
    //添加照相机矩阵
    M3DMatrix44f mCamera;
    //从camraFrame中获取一个4*4的矩阵
    cameraFrame.GetCameraMatrix(mCamera);
    //矩阵乘以矩阵堆栈顶部矩阵，相乘结果存储到堆栈的顶部 也就是将照相机矩阵 与 当前模型矩阵相乘 压入栈顶
    modelViewMatrix.MultMatrix(mCamera);
    
    //创建mObjectFrame矩阵
    M3DMatrix44f mObjectFrame;
    //从objectFrame中获取矩阵，objectFrame保存的是特殊键位的变换矩阵
    objectFrame.GetMatrix(mObjectFrame);
    //矩阵乘以矩阵堆栈顶部矩阵，相乘结果存储到堆栈的顶部 将世界变换矩阵 与 当前模型矩阵相乘 压入栈顶
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    //绑定纹理，因为我们的项目中只有一个纹理。如果有多个纹理。绑定纹理很重要
    //绑定纹理的时候这里其实是一个数组，如果只写一个，也会认为是数组中的第0个
    glBindTexture(GL_TEXTURE_2D, textureID);
    //在使用前绑定一下，
    
    /**点光源着色器
     参数1：GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF（着色器标签）
     参数2：模型视图矩阵
     参数3：投影矩阵
     参数4：视点坐标系中的光源位置
     参数5：基本漫反射颜色
     参数6：图形颜色（用纹理就不需要设置颜色。设置为0）
     */
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
                                 transformPipeline.GetModelViewMatrix(),
                                 transformPipeline.GetProjectionMatrix(),
                                 vLightPos, vWhite, 0);
    
    //pyramidBatch 绘制
    pyramidBatch.Draw();
    
    //模型视图出栈，恢复矩阵（push一次就要pop一次）
    modelViewMatrix.PopMatrix();
    
    //交换缓存区
    glutSwapBuffers();
}



void SpecialKeys(int key, int x, int y)
{
    if(key == GLUT_KEY_UP)
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_DOWN)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_LEFT)
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    
    glutPostRedisplay();
}





void ChangeSize(int w, int h)
{
    glViewport(0, 0, w, h);
    
    //创建投影矩阵
    viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 500.0f);
  
    //viewFrustum.GetProjectionMatrix()  获取viewFrustum投影矩阵
    //并将其加载到投影矩阵堆栈上
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // 设置变换管道以使用两个矩阵堆栈（变换矩阵modelViewMatrix ，投影矩阵projectionMatrix）
    //初始化GLGeometryTransform 的实例transformPipeline.通过将它的内部指针设置为模型视图矩阵堆栈 和 投影矩阵堆栈实例，来完成初始化
    //当然这个操作也可以在SetupRC 函数中完成，但是在窗口大小改变时或者窗口创建时设置它们并没有坏处。而且这样可以一次性完成矩阵和管线的设置。
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}


int main(int argc, char* argv[])
{
    gltSetWorkingDirectory(argv[0]);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Pyramid");
    glutReshapeFunc(ChangeSize);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    
    SetupRC();
    
    glutMainLoop();
    
    ShutdownRC();
    
    return 0;
}
