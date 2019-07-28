#include"httplib.h"
#include<signal.h>
#include"db.hpp"

MYSQL* mysql = NULL;

int main(){

  using namespace httplib;
  using namespace blog_system;
 
  // 1.先和数据库建立好链接
  mysql = blog_system::MySQLInit(); 
  signal(SIGINT, [](int) {blog_system:: MySQLRelease(mysql); exit(0); });
  // 2.创建相关数据库处理对象
  BlogTable blog_table(mysql);
  TagTable tag_table(mysql);
  // 3.创建服务器，并设置"路由"（HTTP服务器中的路由，和IP协议的路由不一样）。此处的路由指的是把方法+path=>那个处理函数 这样的关联声明清楚
  Server server;

  //新增博客
  server.Post("/blog",[&blog_table](const Request& req,Response& resp){
      printf("新增博客!\n");
      // 1. 获取到请求中的body并解析成 Json
      Json::Reader reader ;
      Json::FastWriter writer;
      Json::Value req_json;
      Json::Value resp_json;
      //第一个参数需要解析的字符串，需要把解析后的字符串放到那里去
     bool ret =  reader.parse(req.body,req_json);
     if (!ret) {
     // 请求解析出错,给用户提示， 返回一个400响应
     printf("解析请求失败! %s\n",req.body.c_str());
     // 构造一个响应对象，告诉客户端出错了
     resp_json["ok"] = false;
     resp_json["reason"] = "parse Request failed!\n";
     resp.status = 400;
     resp.set_content(writer.write(resp_json), "application/json");
      return ;
     }

     // 2. 进行参数校验
     if (req_json["title"].empty() 
         || req_json["content"].empty()
         || req_json["tag_id"].empty()
         || req_json["create_time"].empty()) {
     resp_json["ok"] = false;
     resp_json["reason"] = "Request fields error!\n";
     resp.status = 400;
     resp.set_content(writer.write(resp_json), "application/json");
     return; 
     }

     // 3. 调用数据库接口进行操作数据
     ret = blog_table.Insert(req_json);
     if (!ret) {
       printf("博客插入失败");
       resp_json["ok"] = false;
       resp_json["reason"] = "Insert failed!\n";
       resp.status = 500;
       resp.set_content(writer.write(resp_json), "application/json");
       return;
     }

     // 4. 封装正确的返回结果
     printf("博客插入成功！\n");
     resp_json["ok"] = true;
     resp.set_content(writer.write(resp_json), "application/json");
     return;
     
  });

  //查看所有博客（可以按照标签筛选）
  server.Get("/blog", [&blog_table](const Request& req, Response& resp) {
      printf("查看所有博客!\n");
      Json::FastWriter writer;
      Json::Value resp_json;
      // 如果没传 tag_id, 返回的是空字符串
      const std::string& tag_id = req.get_param_value("tag_id");
      // 对于查看博客来说 API 没有请求参数, 不需要解析参数和校验了, 直接构造结果即可
      // 1. 调用数据库接口查询数据
      bool ret = blog_table.SelectAll(&resp_json, tag_id);
      if (!ret) {
      resp_json["ok"] = false;
      resp_json["reason"] = "SelectAll failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }
      // 2. 构造响应结果
      resp.set_content(writer.write(resp_json), "application/json");
      return;
 });

  //查看一篇博客的具体内容
  server.Get(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp) {
      Json::Value resp_json;
      Json::FastWriter writer;
      // 1. 解析获取 blog_id
      int32_t blog_id = std::stoi(req.matches[1].str());
      printf("查看 id 为 %d 的博客!\n",blog_id);
      // 2. 调用数据库接口查看博客 
      bool ret = blog_table.SelectOne(blog_id, &resp_json);
      if (!ret) { 
      resp_json["ok"] = false;
      resp_json["reason"] = "SelectOne failed:" + std::to_string(blog_id);
      resp.status = 404;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }
      // 3. 包装正确的响应
      resp_json["ok"] = true;
      resp.set_content(writer.write(resp_json), "application/json"); 
      return ;
   });


  // 修改博客
  server.Put(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp) {
      // 1.先获取到博客的id
      int32_t blog_id = std::stoi(req.matches[1].str());
      printf("修改 id 为 %d 的博客!\n",blog_id);
     //  2.获取到请求并解析结果
     Json::Reader reader ;
     Json::FastWriter writer;
     Json::Value req_json;
     Json::Value resp_json;
    //第一个参数需要解析的字符串，需要把解析后的字符串放到那里去
     bool ret =  reader.parse(req.body,req_json);
     if (!ret) {
     // 构造一个响应对象，告诉客户端出错了
     resp_json["ok"] = false;
     resp_json["reason"] = " update blog parse Request failed!\n";
     resp.status = 400;
     resp.set_content(writer.write(resp_json), "application/json");
      return ;
     }
     // 3.校验参数是否符合预期
      if (req_json["title"].empty() 
         || req_json["content"].empty()
         || req_json["tag_id"].empty()){
     resp_json["ok"] = false;
     resp_json["reason"] = "update blog request format error!\n";
     resp.status = 400;
     resp.set_content(writer.write(resp_json), "application/json");
     return; 
     }
     // 4. 调用数据库接口进行修改
     // 从path中得到的 id 设置到Json对象中
     req_json["blog_id"] = blog_id;
     ret = blog_table.Update(req_json); 
     if (!ret) { 
     resp_json["ok"] = false;
     resp_json["reason"] = "Update blog database failed!\n";
     resp.status = 500;
     resp.set_content(writer.write(resp_json), "application/json");
     return;
     }
     // 5. 封装正确的数据
     resp_json["ok"] = true;
     resp.set_content(writer.write(resp_json), "application/json");
     return;
  
      });
  // 删除博客
  //raw string(c++ 11), 转义字符不生效. 用来表示正则表达式正好合适
  // 关于正则表达式, 只介绍最基础概念即可. \d+ 表示匹配一个数字 
  // http://help.locoy.com/Document/Learn_Regex_For_30_Minutes.htm
  server.Delete(R"(/blog/(\d+))", [&blog_table](const Request& req, Response& resp) {
      Json::Value resp_json;
      Json::FastWriter writer;
      int32_t blog_id = std::stoi(req.matches[1].str());
      printf("删除 id 为 %d 的博客!\n",blog_id);
     // 2. 调用数据库操作
     bool ret = blog_table.Delete(blog_id);
     if(!ret){
     resp_json["ok"] = false;
     resp_json["reason"] = "Delete blog  failed!\n";
     resp.status = 500;
     resp.set_content(writer.write(resp_json), "application/json");
     return;
     }
     // 3. 构造正确的响应
     resp_json["ok"] = true;
     resp.set_content(writer.write(resp_json), "application/json");
     return;
      });

  // 新增标签
  server.Post("/tag", [&tag_table](const Request& req, Response& resp) { 
      Json::Reader reader;
      Json::FastWriter writer;
      Json::Value req_json;
      Json::Value resp_json;
      // 1. 请求解析成 Json 格式
      bool ret = reader.parse(req.body, req_json);
      if (!ret) {
      // 请求解析出错, 返回一个400响应
      printf("插入标签失败！1\n");
      resp_json["ok"] = false;
      resp_json["reason"] = "insert tag req parse failed!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return; 
      }

      // 2. 校验标签格式
      if (req_json["tag_name"].empty()){
      printf("插入标签失败！2\n");
      // 请求解析出错, 返回一个400响应
      resp_json["ok"] = false;
      resp_json["reason"] = "insert tag req format error";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
      }


      // 3. 调用数据库接口, 插入标签
      ret = tag_table.Insert(req_json);
      if (!ret) {
        printf("插入标签失败！3\n");
        resp_json["ok"] = false;
        resp_json["reason"] = "insert tag datatbase failed";
        resp.status = 500;
        resp.set_content(writer.write(resp_json), "application/json");
        return;
      }

      // 4. 返回正确的结果 
      resp_json["ok"] = true;
      resp.set_content(writer.write(resp_json), "application/json");
      return ;

      });

  // 删除标签
  server.Delete(R"(/tag/(\d+))", [&tag_table](const Request& req, Response& resp) { 
      Json::Value resp_json;
      Json::FastWriter writer;
      // 1. 解析出 tag_id
      int32_t tag_id = std::stoi(req.matches[1]);
      printf("删除 id为 %d 的标签!\n",tag_id);
      // 2. 执行数据库操作删除标签
       bool ret = tag_table.Delete(tag_id);
       if (!ret) {
       printf("删除标签失败!\n");
       resp_json["ok"] = false;
       resp_json["reason"] = "TagTable delete failed!\n";
       resp.status = 500;
       resp.set_content(writer.write(resp_json),"application/json");
       return;
       }
       // 3. 包装正确的结果
       resp_json["ok"] = true;
       resp.set_content(writer.write(resp_json), "application/json");
       return;
       }
       );

  // 获取所有标签
       server.Get("/tag", [&tag_table](const Request& req, Response& resp) {
          Json::Reader reader;
          Json::FastWriter writer;
          Json::Value resp_json;
          // 1.不需要解析参数，直接 调用数据库接口查询数据
          bool ret = tag_table.SelectAll(&resp_json);
          if (!ret) {
          printf("获取所有标签失败 !\n");
          resp_json["ok"] = false;
          resp_json["reason"] = "get all tags failed!\n";
          resp.status = 500;
          resp.set_content(writer.write(resp_json), "application/json");
          return;
          }
          // 2. 构造响应结果
          resp.set_content(writer.write(resp_json), "application/json");
      });
  // 设置静态文件目录
  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0", 9093);
 return 0;
} 
