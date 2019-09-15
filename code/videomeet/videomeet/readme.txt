this is the videomeet
we should learn more`
try connect
»·¾³ÅäÖÃwordÖÐÊÇÅäÖÃffmpeg²å¼þÒÔ¼°flask¿âµÄ·½·¨

config.json 保存的是终端的一些配置信息

1、IP是以99开头的IP是与双向站联通的外部IP

2、编码器ID是视频终端的编号，这个是唯一的，用于识别不同的视频终端(终端)，也用于在中心站处根据ID进行信道优先级分配。

3、channel_x = decode_id 表示的是从第x个信道分发下来的视频数据使用第 decode_id 个解码器解码！

id2ip_de.json 保存的是编码器、解码器的编号和内部IP对应的关系

ch2ip.json 信道与解码器对应的关系

