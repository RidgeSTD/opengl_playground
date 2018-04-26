//
//  main.cpp
//  OpenGLDemo
//
//  Created by 919575700@qq.com on 16/4/24.
//  Copyright © 2016年 Jiangxh. All rights reserved.
//
#include <GLUT/GLUT.h> // GLUT窗口库

// 自定义一个绘制函数
void display()
{
    // 清空
    glClear(GL_COLOR_BUFFER_BIT);
    // 开始绘制一个多边形
    glBegin(GL_POLYGON);
    // 多边形的顶点（三角形）
    glVertex2f(-0.5, -0.5);
    glVertex2f(0, 0.5);
    glVertex2f(0.5, -0.5);
    //绘制结束
    glEnd();
    glFlush();
}

// 程序入口
int main(int argc, char ** argv)
{
    //窗口初始化
    glutInit(&argc, argv);
    //创建一个名为Demo的窗口
    glutCreateWindow("Demo");
    //绘图函数
    glutDisplayFunc(display);
    glutMainLoop();
}