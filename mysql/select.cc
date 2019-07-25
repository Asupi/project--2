#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>

int main(){
    //1.创建一个数据库连接的句柄（类似一个操控数据库的遥控器）
    MYSQL* connect_fd = mysql_init(NULL);
    //2.和数据库建立连接（应用层角度的建立连接，和Tcp区分开）
    //  连接过程需要指定一些必要的信息
    //  a）连接句柄
    //  b）服务器的Ip地址
    //  c）数据库的用户名
    //  d）数据库密码
    //  e）数据库名（blog_system）
    //  f）服务器的端口号
    //  g）unix_sock NULL 
    //  h）client_flag 0
    if((mysql_real_connect(connect_fd,"127.0.0.1","root",
                    "sp171213","blog_system",3306,NULL,0)==NULL)){
            printf("连接失败！%s\n",mysql_error(connect_fd));
            }
    printf("连接成功！\n");
    //3. 设置编码方式
    //   mysql server 部分最初安装的时候已经设置成了 utf8
    //   也得在客户端这边设成 utf8
    mysql_set_character_set(connect_fd,"utf8");
    //4. 拼接SQL语句
    char sql[1024 * 4]= "select * from blog_table";
    //5. 执行Sql语句
      int ret = mysql_query(connect_fd,sql);
    if(ret < 0){
        printf("mysql_query failed! %s\n",mysql_error(connect_fd));
        mysql_close(connect_fd);
        return 1;
    }
    //6. 遍历结果集合
    MYSQL_RES* result = mysql_store_result(connect_fd);
    // a) 获取到结果集合中的行数和列数
    int rows = mysql_num_rows(result);
    int fields = mysql_num_fields(result);
    // b)根据行数和列数来遍历结果
    for(int i=0; i<rows; ++i){
      //一次获取到一行的数据
      MYSQL_ROW row = mysql_fetch_row(result);
      for(int j =0;j < fields;++j){
          printf("%s\t",row[j]);
      }
      printf("\n");
    }
    //释放结果集合，容易遗忘
    mysql_free_result(result);

    //断开连接
    mysql_close(connect_fd);
  
    return 0;
}
