#!/usr/bin/python
#  -*- coding:utf-8 -*-
import sys
# print "python ............"
import numpy as np
import os, base64
reload(sys)
sys.setdefaultencoding('utf8')

sys.path.append("/usr/local/lib/python2.6/site-packages/")
import cv2
print cv2.__file__

class base64transform(object):
    """
    base64 转码
    """

    def __init__(self):
        """
        purpose:构造函数 s
        """
        #print "python ............."
        self.LOCAL_PATH='./imgs'

    def create_file_path(self, item_id):
        """
        purpose:创建本地保存文件路径
        input:item_id
        output:文件路径
        """
        # 文件全路径
        file_path = self.LOCAL_PATH + str(item_id) + '.jpg'

        # 目录是否存在，不存在重新建立
        path = '/'.join(file_path.split('/')[:-1])
        isExists = os.path.exists(path)
        if not isExists:
            os.makedirs(path)
        return file_path

    def transform(self):
        """
        读取文件进行base16转码
        """
        print sys.argv[1]
        inputRead = open(sys.argv[1], 'r')
        output_result = open(sys.argv[2], 'w+')
        #outputid =open(sys.argv[2], 'w+')
        #outputimgs = open(sys.argv[3], 'w+')
        #outputtime = open(sys.argv[4], 'w+')
        outputimgfile =  sys.argv[3]
        self.LOCAL_PATH=outputimgfile

        print "############python 转码。。"
        print "############"

        line = inputRead.readline()  # 调用文件的 readline()方法
        img_num=0
        while line:
            #print line
            img_num= img_num+1
            # if img_num> 5:
            #     break
            item_id = line.split('\t')[0]
            timeint = line.split('\t')[1]
            imagebase64 = line.split('\t')[2][:-1]
            # print  "line : " + line
            try:
                imgData = base64.b64decode(imagebase64)
                nparr = np.fromstring(imgData, np.uint8)
                img_np = cv2.imdecode(nparr, cv2.CV_LOAD_IMAGE_COLOR)
                file_path = self.create_file_path(item_id)
                #print file_path
                cv2.imwrite(file_path, img_np)
                # plt.imshow(img_np)
                # plt.savefig
                # //plt.show()
                output_result.write(item_id+'\t')
                output_result.write(timeint+'\t')
                output_result.write(file_path + '\n')

            except Exception, e:
                print Exception, ":", e
            line = inputRead.readline()
        print("#######################转换图片数目: "+str(img_num))
        output_result.close()
        inputRead.close()
        return True


if __name__ == '__main__':
    main = base64transform()
    re=main.transform()
    if(re):
        sys.exit(0)
    else :
        sys.exit(1)
