#include"db.hpp"



//使用这个程序来测试刚才封装得 MYSQL 操作是否正确
//单元测试
/*void TsetBlogTable(){
  Json::StyledWriter writer;
  MYSQL* mysql = blog_system::MySQLInit();
  blog_system::BlogTable blog_table(mysql);
  bool ret = false;
  Json::Value blog;

  blog["title"] = "初识 C 语言"; 
   blog["content"] = "人生巅峰";
   blog["tag_id"] = 1;
   blog["create_time"] = "2019/07/28";
   bool ret = blog_table.Insert(blog);
   printf("insert: %d\n",ret);

  //测试查找
  Json::Value blogs;
  ret=blog_table.SelectAll(&blogs);
  printf("select add %d\n",ret);
  printf("blogs:%s\n",writer.write(blogs).c_str());
  
  //测试查找单个博客
  ret = blog_table.SelectOne(1,&blog);
  printf("select one %d\n",ret);
  printf("blog: %s\n",writer.write(blog).c_str());


  //测试修改博客
  blog["blog_id"] = 1; 
  blog["title"] = "测试更新博客";
  blog["content"] = "1. 变量和类型\n什么事变量？创建一个'变量'街相当于创建一块内存空间";
  blog["tag_id"] = 2; 
  blog["create_time"] = "2019/07/28 12:00"; 
  ret = blog_table.Update(blog);
  printf("update %d\n",ret);

  //测试删除博客
  ret = blog_table.Delete(1);
  printf("delete %d\n",ret);

  blog_system::MySQLRelease(mysql);
}
*/
void TestTagTable(){
  Json::StyledWriter writer;
  MYSQL * mysql = blog_system::MySQLInit();
  blog_system::TagTable tag_table(mysql);
/*
  Json::Value tag;
  tag["tag_name"]="C语言";
  bool ret = tag_table.Insert(tag);
  printf("insert %d\n",ret);

  //测试查找标签
  Json::Value tags;
  bool ret = tag_table.SelectAll(&tags);
  printf("select all %d\n",ret);
  printf("tags:%s\n",writer.write(tags).c_str());
*/
  
  //测试删除标签
  bool ret = tag_table.Delete(1);
  printf("delete %d\n",ret);
  blog_system::MySQLRelease(mysql);

}
int main(){
 // TsetBlogTable();
  TestTagTable();
  return 0;
}
