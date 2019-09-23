# WindowlessRichEdit
使用windowless rich edit controls的例子

搜遍全网没有好办法去掉超链接的下划线（如果有人找到办法麻烦分享给我，万分感谢），
目前唯一的解决方法是不用ms的en_link,[自己实现颜色设置和动作触发](https://bbs.csdn.net/topics/310060080)，点击触发(本文代码中已经给出简单实现)

# windows rich text 富文本相关

需求是在自己的无边框窗口上显示富文本，自己完全用gdi实现工作量太大；使用win32控件rich edit control和我得无边框窗口冲突，显示不了；最终采用的方法是windowless richedit将文字绘制在我的无边框窗口dc上（遇到一个问题，richedit的绘制是没有alpha透明通道的，所以绘制到我的无边框窗口上会有文字穿透的效果，解决方法是绘制文字前备份alpha，绘制完再恢复（注意GetObject函数不能拿到CreateCompatibleBitmap创建的位图的内存数据，所以[位图需要用CreateDIBSection来创建](https://blog.csdn.net/what951006/article/details/79106306)））

[富文本服务](https://docs.microsoft.com/zh-cn/windows/win32/controls/windowless-rich-edit-controls)

[实现一个可编辑的Windowless RichEdit](http://zplutor.github.io/2015/12/06/implement-an-editable-windowless-richedit/)
[Windowless RichEdit](https://www.codeproject.com/Articles/15906/Using-Windowless-RichEdit-Controls)

[我的RichEdit控件可以包含可点击的链接吗？](https://stackoverflow.com/questions/45366016/can-my-richedit-control-contain-clickable-links)

[rich edit友好名称超链接](https://blogs.msdn.microsoft.com/murrays/2009/09/24/richedit-friendly-name-hyperlinks/)

[EM_AUTOURLDETECT](https://docs.microsoft.com/zh-cn/windows/win32/controls/em-autourldetect?redirectedfrom=MSDN)

[CRichEditControl50W](https://www.codeproject.com/Articles/9810/CRichEditControl50W-A-VC-Rich-Text-Edit-4-1-MFC-Co)

[windowless-richedit](http://zplutor.github.io/2015/11/22/create-a-simplest-windowless-richedit/)

[rich edit controls](https://docs.microsoft.com/en-us/windows/win32/controls/using-rich-edit-controls)

[GDIplus draw rich text](https://www.codeproject.com/Articles/1259387/Drawing-Rich-Text-with-GDIplus)

[MeasureString  DrawString](https://social.msdn.microsoft.com/Forums/en-US/976d049b-8625-4678-93f5-c8c658c93a5d/create-different-text-color-with-gdi?forum=vcgeneral)

[ITextSevices--Using-the-Windowless-RichEdit-for-Drawing-RTF](https://www.codeguru.com/cpp/controls/richedit/windowless/article.php/c5367/ITextSevices--Using-the-Windowless-RichEdit-for-Drawing-RTF.htm)

[COM-Interface-Hooking-and-Its-Application-Part-I](https://www.codeproject.com/Articles/5253/COM-Interface-Hooking-and-Its-Application-Part-I)

[MSN windowless RichEdit 控件](https://blog.csdn.net/happyhell/article/details/5247581)

[位置计算相关](http://www.voidcn.com/article/p-gfdtgvwa-bqa.html)

[自己实现超链接（无法去掉下划线）](https://bbs.csdn.net/topics/310060080)

