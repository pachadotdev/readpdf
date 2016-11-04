#include "tesseract_types.h"
#include <genericvector.h>

typedef GenericVector<STRING> stringvec;

// [[Rcpp::export]]
Rcpp::List tesseract_config(){
  return Rcpp::List::create(
    Rcpp::_["version"] = tesseract::TessBaseAPI::Version()
  );
}

// [[Rcpp::export]]
TessPtr tesseract_engine_internal(Rcpp::CharacterVector datapath, Rcpp::CharacterVector language){
  const char * path = NULL;
  const char * lang = NULL;
  if(datapath.length())
    path = CHAR(STRING_ELT(datapath, 0));
  if(language.length())
    lang = CHAR(STRING_ELT(language, 0));
  tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
  if (api->Init(path, lang))
    throw std::runtime_error("Could not initialize tesseract");
  TessPtr ptr(api);
  ptr.attr("class") = Rcpp::CharacterVector::create("tesseract");
  return ptr;
}

tesseract::TessBaseAPI * get_engine(TessPtr engine){
  tesseract::TessBaseAPI * api = engine.get();
  if(api == NULL)
    throw std::runtime_error("pointer is dead");
  return api;
}

// [[Rcpp::export]]
Rcpp::List engine_info_internal(TessPtr ptr){
  tesseract::TessBaseAPI * api = get_engine(ptr);
  GenericVector<STRING> * langs = new GenericVector<STRING>;
  api->GetAvailableLanguagesAsVector(langs);
  Rcpp::CharacterVector available = Rcpp::CharacterVector::create();
  for(int i = 0; i < langs->length(); i++)
    available.push_back(langs->get(i).c_str());
  delete langs;
  langs = new GenericVector<STRING>;
  api->GetLoadedLanguagesAsVector(langs);
  Rcpp::CharacterVector loaded = Rcpp::CharacterVector::create();
  for(int i = 0; i < langs->length(); i++)
    loaded.push_back(langs->get(i).c_str());
  delete langs;
  return Rcpp::List::create(
    Rcpp::_["datapath"] = api->GetDatapath(),
    Rcpp::_["loaded"] = loaded,
    Rcpp::_["available"] = available
  );
}

// [[Rcpp::export]]
Rcpp::String ocr_raw(Rcpp::RawVector input, TessPtr ptr){
    tesseract::TessBaseAPI *api = get_engine(ptr);

    // Open input image with leptonica library
    Pix *image =  pixReadMem(input.begin(), input.length());
    if(!image)
      throw std::runtime_error("Failed to read image");

    // Get OCR result
    api->SetImage(image);
    char *outText = api->GetUTF8Text();
    pixDestroy(&image);
    api->Clear();

    // Destroy used object and release memory
    Rcpp::String y(outText);
    y.set_encoding(CE_UTF8);
    delete [] outText;
    return y;
}

// [[Rcpp::export]]
Rcpp::String ocr_file(std::string file, TessPtr ptr){
  tesseract::TessBaseAPI *api = get_engine(ptr);

  // Open input image with leptonica library
  Pix *image =  pixRead(file.c_str());
  if(!image)
    throw std::runtime_error("Failed to read image");

  // Get OCR result
  api->SetImage(image);
  char *outText = api->GetUTF8Text();
  pixDestroy(&image);
  api->Clear();

  // Destroy used object and release memory
  Rcpp::String y(outText);
  y.set_encoding(CE_UTF8);
  delete [] outText;
  return y;
}