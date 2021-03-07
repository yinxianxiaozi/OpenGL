/*
 通过该工程学习简单的画面渲染流程。主要是进行流程的认识
 
 1、在main函数中创建背景图、添加回调函数
 2、SetupRC方法中使用简单批次容器进行三角形图元的设置
 3、ChangeSize方法中设置OpenGL视口（视口就是投影到屏幕上的可视范围，一般就跟屏幕大小一样就行了）
 4、RenderScene方法中使用固定管线将图像渲染到屏幕上
 
 
 GLShaderManager.h是着色器管理类，1、可以创建并管理着色器；2、还提供了一组存储着色器
 GLTools.h包含了很多的类似C语言的独立的函数，使用这里的函数进行操作
 <glut/glut.h>
 */


#include "GLShaderManager.h"

#include "GLTools.h"

#include <glut/glut.h>

//定义一个简单的批次容器，是GLTools的简单容器类 ，用来管理图元的，这里先只管理顶点
GLBatch triangleBatch;

//定义一个着色器管理类
GLShaderManager shaderManager;

/*
 1、使用前需要glutReshapeFunc(函数名)注册为重塑函数
 2、0,0代表窗口中视口的左下角坐标，w，h代表像素
 3、自定义函数，名字可以随便起，只要是通过glutReshapeFunc注册就可以
 
 触发条件：
    当窗口大小发生时改变
    第一次创建窗口的时候会调用该函数
 
 处理业务：
    设置OpenGL视口
    设置OpenGL投影方式
 */

void ChangeSize(int w,int h)

{
    //参数就和iOS中所用的参数一样，前面两个是坐标，后面两个是宽度和高度
    glViewport(0,0, w, h);
    
}

//为程序作一次性的设置

/*
 1、设置需要渲染的图形的数据准备工作，比如顶点数据、颜色数据等数据
 2、自定义函数，名字可以随便写
 
 触发条件：
    在main函数中手动调用
 
 处理业务：
    1、设置窗口背景颜色
    2、初始化存储着色器ShaderManager
    3、设置图像顶点数据
    4、将数据传递到着色器（利用GLBatch三角形批次类）
 */

void SetupRC()

{
    
    //设置背景颜色，设置到颜色缓冲区
    
    glClearColor(0.0f,0.0f,1.0f,1.0f);
    
    //初始化着色管理器
    
    shaderManager.InitializeStockShaders();
    
    //设置三角形，其中数组vVert包含所有3个顶点的x,y,2D笛卡尔坐标系。
    
    GLfloat vVerts[] = {
        
        -0.5f,0.0f,0.0f,
        
        0.5f,0.0f,0.0f,
        
        0.0f,0.5f,0.0f,
        
    };
    
    /*
     建立一个三角形的批次
     */
    
    triangleBatch.Begin(GL_TRIANGLES,3);//图像的连接方式，GL_TRIANGLES表示是三角形，3表示顶点是三个顶点
    
    triangleBatch.CopyVertexData3f(vVerts);//copy顶点数据，也就是添加数据
    
    triangleBatch.End();//动作完成
    
}

//开始渲染

/*
 1、渲染函数，当窗口发生改变，或开发者主动渲染的时候会调用该函数来渲染界面
 2、需要glutDisplayFunc(函数名)来注册为渲染函数
 3、自定义函数，名字可以随便写，只要通过glutDisplayFunc注册即可
 
触发条件：
    系统自动触发
    开发者手动调用函数触发
 
 处理业务：
    1、清除缓存区（颜色、深度、模板缓存区等）
    2、使用存储着色器
    3、绘制图像
 */

void RenderScene(void)

{
    
    /*
     1、清除一个或一组特定的缓冲区
     */
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    /*
     2、使用存储着色器
     */
    
    //设置一组浮点数来表示红色
    
    GLfloat vRed[] = {1.0f,0.0f,0.0f,1.0f};
    
    //传递到存储着色器，即单元着色器（GLT_SHADER_IDENTITY着色器），这个着色器只是使用指定颜色以默认笛卡尔坐标第在屏幕上渲染几何图形
    //UseStockShader使用固定管线
    shaderManager.UseStockShader(GLT_SHADER_IDENTITY,vRed);
    
    //提交着色器，表示可以绘制了，并不会直接绘制
    triangleBatch.Draw();
    
    /*
     3、将在后台缓冲区进行渲染，然后在结束时交换到前台,
     */
    
    glutSwapBuffers();
    
}

/*
 1、窗口初始化
 2、注册回调函数（窗口大小改变、窗口颜色渲染）
 3、图像数据的准备工作，之后开启循环执行
 */

int main(int argc,char* argv[])

{
    
    //设置当前工作目录，针对MAC OS X
    
    gltSetWorkingDirectory(argv[0]);
    
    /*
     窗体的初始化，这应该就是MAC中的窗口的简单使用吧
     */
    
    //初始化GLUT库
    
    glutInit(&argc, argv);
    
    /*初始化双缓冲窗口，其中标志GLUT_DOUBLE、GLUT_RGBA、GLUT_DEPTH、GLUT_STENCIL分别指
     
     双缓冲窗口、RGBA颜色模式、深度测试、模板缓冲区*/
    //告诉GLUT在创建窗体时使用哪种模式显示
    
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //GLUT窗口大小，标题窗口
    
    glutInitWindowSize(800,600);
    
    glutCreateWindow("Triangle");
    
    /*
     注册回调函数
     */
    //窗口大小改变时调用或第一次创建窗口时调用
    glutReshapeFunc(ChangeSize);
    //屏幕发生变化或主动渲染界面时调用
    glutDisplayFunc(RenderScene);
    
    //驱动程序的初始化中没有出现任何问题。
    
    GLenum err = glewInit();//初始化OpenGL驱动程序
    
    if(GLEW_OK != err) {
        
        fprintf(stderr,"glew error:%s\n",glewGetErrorString(err));
        
        return 1;
        
    }
    
    //调用SetupRC进行一些初始化设置，顶点数据、颜色数据
    
    SetupRC();
    
    //循环执行，类似于iOS中的Runloop运行循环
    glutMainLoop();
    
    return 0;
    
}

