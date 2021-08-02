#pragma once

#include "Sock.hpp"
#include "Log.hpp"
#include "Util.hpp"
#include "unordered_map"

// /a/b/c.html
#define WEBROOT "wwwroot"
#define HOMEPAGE "fishjoy.html"
#define VERSION "HTTP/1.0"

static std::string CodeToDesc(int code)
{
  std::string desc;
  switch (code){
    case 200:
      desc = "OK";
      break;
    case 404:
      desc = "Not Found";
      break;
    default:
      desc = "OK";
      break;
  }

  return desc;
}

static std::string SuffixToDesc(const std::string &suffix)
{
  if (suffix == ".html" || suffix == ".htm"){
    return "text/html";
  }
  else if (suffix == ".js"){
    return "application/x-javascript";
  }
  else if (suffix == ".css"){
    return "text/css";
  }
  else if (suffix == ".jpg"){
    return "image/jpeg";
  }
  else{
    return "text/html";
  }
}

class HttpRequest{
  private:
    std::string request_line;
    std::vector<std::string> request_header;
    std::string blank;
    std::string request_body;
  private:
    std::string method;
    std::string uri;
    std::string version;

    std::string path;
    std::string query_string;

    std::unordered_map<std::string, std::string> header_kv;

    ssize_t content_length;

    bool cgi;

    ssize_t file_size;

    std::string suffix;
  public:
    HttpRequest()
      :blank("\n"), content_length(-1), \
       path(WEBROOT), cgi(false), suffix("text/html")
  {}
    void SetRquestLine(const std::string &line)
    {
      request_line = line;
    }
    void InsertHeaderLine(const std::string &line)
    {
      request_header.push_back(line);
      //LOG(Notice, line);
    }
    void RquestLineParse()
    {
      Util::string_parse(request_line, method, uri, version);
      // LOG(Notice, request_line);
       LOG(Notice, method);
       LOG(Notice, uri);
       LOG(Notice, version);
    }
    void RequestHeaderParse()
    {
      for (auto it = request_header.begin(); it != request_header.end(); it++){
        std::string k, v;
        Util::get_request_hander_kv(*it, k, v);
        //LOG(Notice, k);
        //LOG(Notice, v);
        if (k == "Content-Length"){
          content_length = atoi(v.c_str());
        }
        header_kv.insert({ k, v });
      }
    }
    bool IsNeedRecvBody()
    {
      //Post,PoSt, POSt
      if (strcasecmp(method.c_str(), "POST") == 0 && content_length > 0){
        cgi = true;
        return true;
      }

      return false;
    }
    ssize_t GetContentLength()
    {
      return content_length;
    }
    void SetRequestBody(const std::string &body)
    {
      request_body = body;
    }
    bool IsMethodLegal()
    {
      if (strcasecmp(method.c_str(), "POST") == 0 || \
          strcasecmp(method.c_str(), "GEt") == 0){
        return true;
      }
      return false;
    }
    bool IsGet()
    {
      return strcasecmp(method.c_str(), "GET") == 0 ;
    }
    bool IsPost()
    {
      return strcasecmp(method.c_str(), "POST") == 0 ;
    }
    void GetUriParse()
    {
      //uri存在
      std::size_t pos = uri.find('?');
      if (pos == std::string::npos){
        path += uri;
      }
      else{
        path += uri.substr(0, pos);
        query_string = uri.substr(pos + 1);
        //cgi = true;
      }
    }
    void PostUriEqPath()
    {
      path += uri;
    }

    void IsAddHomePage()
    {
      //path / or /a/b/c.html or /s
      if (path[path.size() - 1] == '/'){
        path += HOMEPAGE;
      }
    }
    std::string GetPath()
    {
      return path;
    }
    void SetPath(std::string _path)
    {
      path = _path;
    }
    void SetCgi()
    {
      cgi = true;
    }
    bool IsCgi()
    {
      return cgi;
    }
    void SetFileSize(ssize_t s)
    {
      file_size = s;
    }
    ssize_t GetFileSize()
    {
      return file_size;
    }
    std::string GetQueryString()
    {
      return query_string;
    }
    std::string GetBody()
    {
      return request_body;
    }
    std::string GetMethod()
    {
      return method;
    }
    std::string MakeSuffix()
    {
      std::string suffix;
      std::size_t pos = path.rfind(".");
      if (std::string::npos != pos){
        suffix = path.substr(pos);
      }
      return suffix;
    }
    ~HttpRequest()
    {}
};
class HttpResponse{
  private:
    std::string status_line;
    std::vector<std::string> response_header;
    std::string blank;
    std::string response_body;
  public:
    HttpResponse() :blank("\r\n")
  {}
    void SetStatusLine(const std::string &sline)
    {
      status_line = sline;
    }
    std::string GetStatusLine()
    {
      return status_line;
    }
    const std::vector<std::string>& GetRspHeader()
    {
      return response_header;
    }
    void AddHeader(const std::string &ct)
    {
      response_header.push_back(ct);
    }
    ~HttpResponse()
    {}
};
class EndPoint{
  private:
    int sock;
    HttpRequest req;
    HttpResponse rsp;
  private:
    void GetRequestLine()
    {
      std::string line;
      sock::GetLine(sock, line);
      req.SetRquestLine(line);
      req.RquestLineParse();
    }
    void SetResponseStatusLine(int code)
    {
      std::string status_line;
      status_line = VERSION;
      status_line += " ";
      status_line += std::to_string(code);
      status_line += " ";
      status_line += CodeToDesc(code);
      status_line += "\r\n";

      rsp.SetStatusLine(status_line);
    }
    void GetRequestHeader()
    {
      std::string line;
      do{
        line = "";
        sock::GetLine(sock, line);
        req.InsertHeaderLine(line);
      } while (!line.empty());

      req.RequestHeaderParse();
    }
    void GetRequestBody()
    {
      ssize_t len = req.GetContentLength();
      char c;
      std::string body;
      while (len){
        recv(sock, &c, 1, 0);
        body.push_back(c);
        len--;
      }
      req.SetRequestBody(body);
    }

    void SetReaponseHeaderLine()
    {
      std::string suffix = req.MakeSuffix();
      std::string content_type = "Content-Type: ";
      content_type += SuffixToDesc(suffix);
      content_type += "\r\n";
      rsp.AddHeader(content_type);
    }
  public:
    EndPoint(int _sock) :sock(_sock)
  {}
    void RecvRequest()
    {
      //获取完整http请求
      //分析http请求
      //读取并分析完毕第一行
      GetRequestLine();
      //读取报头 && blank
      GetRequestHeader();
      //是否需要读取正文
      if (req.IsNeedRecvBody()){
        //yes
        GetRequestBody();
      }
      //已经读完了所有的请求

    }
    void MakeResponse()
    {
      //开始分析
      //GET and POST
      int code = 200;
      std::string path;
      if (!req.IsMethodLegal()){
        LOG(Warning, "method is not legal!");
        code = 404;
        goto end;
      }
      if (req.IsGet()){
        req.GetUriParse();
      }
      else{
        //POST
        req.PostUriEqPath();
      }
      req.IsAddHomePage();            //如果指明访问路径，则添加首页返回。
      //如下三种情况
      //get && 没有参数 -> path
      //get && 有参数   -> path && query_string
      //POST -> uri -> path && body
      path = req.GetPath();
      LOG(Notice, path);
      struct stat st;
      if (stat(path.c_str(), &st) < 0){
        cout<<path.c_str()<<endl;
        LOG(Warning, "html is not exist! 404");
        code = 404;
        goto end;
      }
      else{
        if (S_ISDIR(st.st_mode)){
          // /a/b/c/index.html
          path += "/";
          req.SetPath(path);
          req.IsAddHomePage();
        }
        else{
          if ((st.st_mode & S_IXUSR) || \
              (st.st_mode & S_IXGRP) || \
              (st.st_mode & S_IXOTH)){
            req.SetCgi();
          }
          else{
            //正常的网页请求
            //Do Nothing
          }
        }
        if (!req.IsCgi()){
          req.SetFileSize(st.st_size);
        }
      }
end:
      //制作response
      SetResponseStatusLine(code);
      SetReaponseHeaderLine();
      //TODO
    }
    void ExecNonCgi(const std::string &path)
    {
      ssize_t size = req.GetFileSize();
      int fd = open(path.c_str(), O_RDONLY);
      if (fd < 0){
        LOG(Error, "path is exist bug!!!");
        return;
      }
      sendfile(sock, fd, nullptr, size);            //此函数把fd里的内容拷贝给了sock，不用再从内核拷至用户空间，用户空间拷至内核空间。
      close(fd);
    }

    //事先做好约定，
    //1.cgi程序如果需要执行 GET方法，那么从父进程导入到子进程的环境变量（query_string_env）里获取数据          
    //cgi程序如果需要用post方法从body里读数据，那么采用从文件里读数据。
    void ExecCgi()
    {
      std::string content_length_env;
      std::string path = req.GetPath();
      std::string method = req.GetMethod();
      std::string method_env = "METHOD=";
      method_env += method;

      std::string query_string;
      std::string query_string_env;

      std::string body;
      int pipe_in[2] = { 0 };
      int pipe_out[2] = { 0 };

      //站在被调用进程的角度
      pipe(pipe_in);
      pipe(pipe_out);

      putenv((char*)method_env.c_str());                  //注意：父进程的环境变量会传递给子进程，而且不论那种方法cgi程序都需要知道method，所以提前导入。
      pid_t id = fork();
      if (id == 0){
        //child
        close(pipe_in[1]);
        close(pipe_out[0]);

        //由于服务器可能同时为多个client服务，cgi程序不能确定从那个文件描述符读取body，所以作约定。这里重定向。
        dup2(pipe_in[0], 0);        
        dup2(pipe_out[1], 1);

        //exec
        //模仿cgi标准
        //一种通过环境变量给子进程 -> GET -> query_string           //适用于参数占用字节数少的
        //程序替换并不会替换环境变量，只会替换代码和数据。
        //
        //再根据method决定导入query_string_env里的访问参数;还是在body里的访问参数。
        if (req.IsGet()){
          query_string = req.GetQueryString();
          query_string_env = "QUERY_STRING=";
          query_string_env += query_string;
          putenv((char*)query_string_env.c_str());
        }
        //如果是post方法，数据量往往比较大，而且cgi程序并不知道读多少，所以传入content_length_env
        else if (req.IsPost()){
          content_length_env = "CONTENT-LENGTH=";
          content_length_env += std::to_string(req.GetContentLength());
          putenv((char*)content_length_env.c_str());
        }
        else{
          //TODO
        }

        execl(path.c_str(), path.c_str(), nullptr);
        exit(0);                     //如果程序替换成功则不执行这一句，失败则直接退出。
      }
      close(pipe_in[0]);
      close(pipe_out[1]);
      //father
      //第二种通过管道传递给子进程 -> POST -> body
      char c = 'Z';
      if (req.IsPost()){
        body = req.GetBody();
        size_t i = 0;
        for (; i < body.size(); i++){          //将request_body通过管道传给子进程重定向后的管道文件。
          write(pipe_in[1], &body[i], 1);
        }
        LOG(Notice, body);
      }
      ssize_t s = 0;
      do{
        s = read(pipe_out[0], &c, 1);
        if (s > 0){
          send(sock, &c, 1, 0);
        }
      } while (s > 0);

      waitpid(id, nullptr, 0);        //等待子进程退出，防止产生僵尸进程。这里阻塞等待并不 影响服务器效率，因为只是服务器开的一个线程去执行等待任务。
      close(pipe_in[1]);
      close(pipe_out[0]);        //同一个进程里的线程共用一个文件描述符表，线程退出时不会自动关闭线程自己曾经打开的文件描述符。如果不手动关闭，就会造成文件描述符不够用
    }
    void SendResponse()
    {
      std::string line = rsp.GetStatusLine();
      send(sock, line.c_str(), line.size(), 0);
      const auto &header = rsp.GetRspHeader();
      auto it = header.begin();
      for (; it != header.end(); it++){
        send(sock, it->c_str(), it->size(), 0);
      }
      send(sock,"\r\n",2,0);            //手动发送响应的空行      

      if (req.IsCgi()){
        //yes->cgi
        LOG(Notice, "use cgi model!");
        ExecCgi();
      }
      else{
        LOG(Notice, "use non cgi model!");
        std::string path = req.GetPath();
        ExecNonCgi(path);
      }
    }
    ~EndPoint()
    {
      if (sock >= 0){
        close(sock);
      }
    }
};

class HandleEntry{
  public:
    static void* handle_http(int sock)
    {
      //int sock = *(int*)arg; delete (int*)arg; 

#ifdef DEBUG
      char request[10240];
      recv(sock, request, sizeof(request), 0);
      cout<<request<<endl;
      close(sock);
#else
      EndPoint *ep = new EndPoint(sock);
      ep->RecvRequest();
      ep->MakeResponse();
      ep->SendResponse();

      delete ep;
#endif
      return nullptr;
    }
};
