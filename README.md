# TouchAgent

安卓无冲突触摸事件注入，核心思路就是捕获原触摸设备触摸B报文并整合UDP接收的外源性触摸报文，解决冲突后合并，写入uinput创建的虚拟触控设备中。



## 已知问题：

1. 多个手指（5+）会卡顿
2. UDP接口可能来的不够方便



## 愿景：

1. 希望好心人能解决一下上述问题
2. 欢迎PR