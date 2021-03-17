
//demo OpenGL 裁剪
#include "GLTools.h"
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

//召唤场景
/*
 1、每次设置完颜色都要清除颜色缓冲区，否则后面的颜色会覆盖前面的颜色
 2、在清屏的时候执行裁剪，因此需要在清屏前开启裁剪测试
 */
void RenderScene(void)
{
    //设置清屏颜色为蓝色
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    //1.现在剪成小红色分区
    //(1)设置裁剪区颜色为红色
    glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
    //(2)设置裁剪尺寸
    glScissor(100, 100, 600, 400);
    //(3)开启裁剪测试
    glEnable(GL_SCISSOR_TEST);
    //(4)开启清屏，执行裁剪
    glClear(GL_COLOR_BUFFER_BIT);
    // 2.裁剪一个绿色的小矩形
    //(1).设置清屏颜色为绿色
    glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
    //(2).设置裁剪尺寸
    glScissor(200, 200, 400, 200);
    //(3).开始清屏执行裁剪
    glClear(GL_COLOR_BUFFER_BIT);
    
    //关闭裁剪测试
    glDisable(GL_SCISSOR_TEST);
    
    //强制执行缓存区
    glutSwapBuffers();
}



void ChangeSize(int w, int h)
{
    //保证高度不能为0
    if(h == 0)
        h = 1;
    
    // 将视口设置为窗口尺寸
    glViewport(0, 0, w, h);
}

//程序入口
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800,600);
    glutCreateWindow("OpenGL Scissor");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutMainLoop();
    
    return 0;
}
