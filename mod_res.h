#pragma once
#ifndef _MOD_RES_H_
#define _MOD_RES_H_


char *html ="<html><head>"
            "<style>"
                "table, th, td {border: 1px solid black; border-collapse: collapse;}"
                "td, th {padding: 10px;}"
            "</style>"
          "</head><body>"
          "<div id=\"local_info\"></div>"
          "<div id=\"container\"></div>"
          "<script>\n"
            "function create_data(data){\n"
                "let local_info = document.getElementById(\"local_info\");\n"
                "let table = document.createElement(\"table\");\n"
                "let thead = document.createElement(\"thead\");\n"
                "let tr = document.createElement(\"tr\"); table.width =\"100%\";\n"
                "table.style.fontSize = \"10px\";\n"
                "let th = document.createElement(\"th\");\n" 
                "th.innerText=\"SERVER HOSTNAME\";\n" 
                "tr.appendChild(th);\n" 
                "thead.appendChild(tr);\n" 
                "th = document.createElement(\"th\");\n" 
                "th.innerText = \"SERVER IP\";\n"                 
                "tr.appendChild(th);\n"
                "thead.appendChild(tr);\n" 
                "th = document.createElement(\"th\");\n" 
                "th.innerText=\"SERVER TIME\";\n"
                "tr.appendChild(th);\n"
                "th = document.createElement(\"th\");\n" 
                "th.innerText=\"-------\";\n"
                "tr.appendChild(th);\n"
                "th = document.createElement(\"th\");\n" 
                "th.innerText=\"--------\";\n"
                "tr.appendChild(th);\n"
                "th = document.createElement(\"th\");\n" 
                "th.innerText=\"--------\";\n"
                "tr.appendChild(th);\n"                
                "th = document.createElement(\"th\");\n" 
                "th.innerText=\"--------\";\n"
                "tr.appendChild(th);\n"                
                "thead.appendChild(tr);\n"
                "table.append(tr);\n"
                "tr = document.createElement(\"tr\");\n" 
                "table.appendChild(tr);\n"
                "let td = document.createElement(\"td\");\n"
                "td.innerText = data[\"host_name\"];\n"
                "tr.appendChild(td);\n"
                "td = document.createElement(\"td\");\n"
                "td.innerText = data[\"host_ip\"];\n"
                "tr.appendChild(td);\n"
                "td = document.createElement(\"td\");\n"
                "td.innerText = data[\"local_millisecond\"];\n"
                "tr.appendChild(td);\n"
                "table.appendChild(tr);\n"
                "td = document.createElement(\"td\");\n"
                "td.innerText = data[\"RESER_1\"];\n"
                "tr.appendChild(td);\n"
                "table.appendChild(tr);\n"
                "td = document.createElement(\"td\");\n"
                "td.innerText = data[\"RESER_2\"];\n"
                "tr.appendChild(td);\n"
                "table.appendChild(tr);\n"
                "td = document.createElement(\"td\");\n"
                "td.innerText = data[\"RESER_3\"];\n"
                "tr.appendChild(td);\n"
                "table.appendChild(tr);\n"                
                "td = document.createElement(\"td\");\n"
                "td.innerText = \"--\";\n"
                "tr.appendChild(td);\n"
                "table.appendChild(tr);\n"                
                "if(local_info.childNodes.length > 0){local_info.removeChild(local_info.childNodes[0]);}\n"
                "local_info.appendChild(table)\n"      
                "if(data[\"data\"].length >0){\n"
                    "let container = document.getElementById(\"container\");\n"
                    "let table = document.createElement(\"table\");\n"
                    "let cols = Object.keys(data[\"data\"][0]);\n"         
                    "let thead = document.createElement(\"thead\");\n"
                    "let tr = document.createElement(\"tr\");\n"
                    "table.width=\"100%\";table.style.fontSize =\"10px\";\n"
                    "cols.forEach((item) => {\n"
                        "let th = document.createElement(\"th\");\n"
                        "th.innerText = item;tr.appendChild(th);\n"
                    "});\n"
                    "thead.appendChild(tr);\n"
                    "table.append(tr);\n"
                    "data[\"data\"].forEach((item) => {\n"
                        "let tr = document.createElement(\"tr\");\n"
                        "let vals = Object.values(item);\n"
                        "if(vals[4]>=10){\n"
                            "tr.bgColor =\"Red\";}else{tr.bgColor =\"Green\";\n"
                        "}\n"                        
                    "vals.forEach((elem) => {\n"
                        "let td = document.createElement(\"td\");\n"
                        "td.innerText = elem;tr.appendChild(td);\n"
                    "});\n"
                    "table.appendChild(tr);});\n"
                    "if(container.childNodes.length>0){\n"
                        "container.removeChild(container.childNodes[0])}\n"
                        "container.appendChild(table)\n"
                    "}\n"
        "}\n"                            
        "function convert() {\n"
			"const request = {"
				"method: \"GET\",\n"
				"url: \"/live\",\n"
				"headers: {\n"
					"\"Content-Type\": \"application/json\",\n"
				"},\n"
			"};\n"
			"const xhr = new XMLHttpRequest();"
			"xhr.open(request.method, request.url);"
			"xhr.setRequestHeader(\"Content-Type\", request.headers[\"Content-Type\"]);"
			"xhr.onload = function() {"
			"if (xhr.status === 200) {"
				"const response = JSON.parse(xhr.responseText);"				
				"create_data(response)"
			"} else {"
				"console.error(\"Error:\", xhr.status);"				
			"}};"
			"xhr.send();"
		"}"
         "setInterval(convert, 1000);"
         "</script>"
         "</body></html>";
#endif