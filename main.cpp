#include <iostream>
#include <string>
#include <curl/curl.h>
#include <regex>
#include <fstream>
#include <vector>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <iomanip> // for std::setw and std::setfill

struct QnA
{
    std::string question;
    std::string answer;
    std::string source;
};

// Assume WriteCallback is defined above...
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

// Define a function to escape JSON special characters
std::string escape_json(const std::string &s)
{
    std::ostringstream o;
    for (auto c = s.cbegin(); c != s.cend(); c++)
    {
        switch (*c)
        {
        case '"':
            o << "\\\"";
            break;
        case '\\':
            o << "\\\\";
            break;
        case '\b':
            o << "\\b";
            break;
        case '\f':
            o << "\\f";
            break;
        case '\n':
            o << "\\n";
            break;
        case '\r':
            o << "\\r";
            break;
        case '\t':
            o << "\\t";
            break;
        default:
            if ('\x00' <= *c && *c <= '\x1f')
            {
                o << "\\u"
                  << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
            }
            else
            {
                o << *c;
            }
        }
    }
    return o.str();
}

std::string http_get(const std::string &url)
{
    CURL *curl = curl_easy_init();
    std::string response;

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl); // Perform the request
        curl_easy_cleanup(curl); // Always cleanup
    }

    return response;
}

std::vector<std::string> parse_and_extract_answer_and_link(const std::string &html_content)
{
    std::string answer;
    std::string source_link;
    std::vector<std::string> result;

    // Parse the HTML content into a DOM
    htmlDocPtr doc = htmlReadDoc((xmlChar *)html_content.c_str(), NULL, NULL, HTML_PARSE_RECOVER | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);

    // If the document is parsed without errors
    if (doc)
    {
        // Create a context for XPath evaluation
        xmlXPathContextPtr context = xmlXPathNewContext(doc);
        if (context)
        {
            // Evaluate XPath expression to find the answer within div#qna_only
            xmlXPathObjectPtr answer_result = xmlXPathEvalExpression((xmlChar *)"//div[@id='qna_only']//text()", context);
            if (answer_result)
            {
                xmlNodeSetPtr nodes = answer_result->nodesetval;
                for (int i = 0; i < nodes->nodeNr; ++i)
                {
                    xmlNodePtr cur = nodes->nodeTab[i];
                    xmlChar *content = xmlNodeGetContent(cur);
                    if (content)
                    {
                        answer += (char *)content;
                        xmlFree(content);
                    }
                }
                xmlXPathFreeObject(answer_result);
            }

            // Evaluate XPath expression to find the source link within div.original_source
            xmlXPathObjectPtr link_result = xmlXPathEvalExpression((xmlChar *)"//div[@class='original_source']/a/@href", context);
            if (link_result)
            {
                xmlNodeSetPtr nodes = link_result->nodesetval;
                if (nodes->nodeNr > 0)
                {
                    xmlChar *content = xmlNodeGetContent(nodes->nodeTab[0]);
                    if (content)
                    {
                        source_link = (char *)content;
                        xmlFree(content);
                    }
                }
                xmlXPathFreeObject(link_result);
            }

            // Clean up
            xmlXPathFreeContext(context);
        }
        xmlFreeDoc(doc);
    }

    // Append answer and source link to the result vector
    result.push_back(answer);
    result.push_back(source_link);

    // Return the result vector
    return result;
}

int main()
{
    LIBXML_TEST_VERSION
    CURL *curl;
    CURLcode res;
    std::string responseString;
    std::vector<std::string> questions; // Use a vector to store questions
    std::vector<QnA> qna_pairs;
    // More specific regex to match only the contents of the <a> tag within <h2> tags
    // std::regex question_regex("<h2><a[^>]*>(.*?)</a></h2>");
    std::regex question_regex("<h2><a[^>]+href=\"([^\"]+)\"[^>]*>([^<]+)</a></h2>");

    std::regex page_count_regex("Pg 1 of (\\d+)"); // Regex to find the total number of pages
    int total_pages = 0;

    curl = curl_easy_init();
    if (curl)
    {
        std::string page_count_url = "https://islamqa.org/category/maliki/";
        curl_easy_setopt(curl, CURLOPT_URL, page_count_url.c_str());
        // Set up other CURL options here...
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK)
        {
            std::smatch page_count_match;
            if (std::regex_search(responseString, page_count_match, page_count_regex) && page_count_match.size() > 1)
            {
                total_pages = std::stoi(page_count_match[1].str());
                std::cout << "Total pages found: " << total_pages << std::endl;
            }
            responseString.clear();
        }
        else
        {
            std::cerr << "Failed to get total page count: " << curl_easy_strerror(res) << std::endl;
            return -1; // Exit if we can't get the total page count
        }

        int questionCount = 0;
        for (int page = 1; page <= total_pages; ++page)
        {
            std::cout << "Page Number: " << page << std::endl;
            std::string url;
            if (page == 1)
            {
                url = "https://islamqa.org/category/maliki/";
            }
            else
            {
                url = "https://islamqa.org/category/maliki/page/" + std::to_string(page) + "/";
            }
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            // Set up other CURL options here...
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

            // Set up other options such as CURLOPT_WRITEDATA, CURLOPT_FOLLOWLOCATION, etc.

            // Clear the response string for each page
            responseString.clear();
            res = curl_easy_perform(curl);

            if (res == CURLE_OK)
            {
                std::smatch matches;
                // Use regex to search for and extract the question text
                while (std::regex_search(responseString, matches, question_regex))
                {
                    std::string link = matches[1].str(); // This is the captured question text
                    std::string question = matches[2].str();
                    std::cout << "Question: " << question << std::endl;
                    std::cout << "Link: " << link << std::endl;
                    std::string answer_html = http_get(link);
                    std::vector<std::string> answer_and_link = parse_and_extract_answer_and_link(answer_html);
                    std::cout << "Answer: " << answer_and_link[0] << std::endl;
                    // Erase up to the end of the matched <h2> tag to continue the search
                    qna_pairs.push_back({question, answer_and_link[0], answer_and_link[1]});
                    responseString = matches.suffix().str();
                }
            }
            else
            {
                // Handle errors
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            }
        }

        // Always clean up
        curl_easy_cleanup(curl);
    }

    // Now, serialize the questions to a JSON string
    std::string jsonString = "{\"Maliki\": [\n";
    for (const auto &qna : qna_pairs)
    {
        jsonString += "\t{\"Question\": \"" + escape_json(qna.question) +
                      "\", \"Answer\": \"" + escape_json(qna.answer) +
                      "\", \"Source\": \"" + escape_json(qna.source) + "\"},\n";
    }
    if (!jsonString.empty())
    {
        jsonString.pop_back(); // Remove the trailing comma
        jsonString.pop_back(); // Remove the newline
    }
    jsonString += "\n]}";

    // Write the JSON string to a file
    std::ofstream jsonFile("maliki_questions.json");
    if (jsonFile.is_open())
    {
        jsonFile << jsonString;
        jsonFile.close();
        std::cout << "Questions saved to maliki_questions.json" << std::endl;
    }
    else
    {
        std::cerr << "Failed to open file for writing." << std::endl;
    }

    return 0;
}
