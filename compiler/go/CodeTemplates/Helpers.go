package TemplateFunctions

import (
	"html/template"
	txtTemplate "text/template"
	"strings"
)
import "github.com/Masterminds/sprig/v3"

// Receives multiple func maps and merges them.
// In case of duplicated keys, only the first key is placed
// The SDK funcs and "spring" funcs are added last
func RunTemplate(name string, templateTxt string, params interface{}, funcs ...map[string]any) (string, error) {
	
	actualFuncs := make(map[string]any)
	
	for _, curFuncs := range funcs {
		for k, v := range curFuncs {
			if _, exists := actualFuncs[k]; !exists {
				actualFuncs[k] = v
			}
		}
	}
	
	for k, v := range templatesFuncMap {
		if _, exists := actualFuncs[k]; !exists {
			actualFuncs[k] = v
		}
	}
	
	for k, v := range sprig.FuncMap() {
		if _, exists := actualFuncs[k]; !exists {
			actualFuncs[k] = v
		}
	}
	
	tmp, err := template.New(name).Funcs(actualFuncs).Parse(templateTxt)
	if err != nil {
		return "", err
	}
	
	buf := strings.Builder{}
	err = tmp.Execute(&buf, params)
	
	if err != nil {
		return "", err
	}
	
	return buf.String(), nil
}


func RunTemplateTxt(name string, templateTxt string, params interface{}, funcs ...map[string]any) (string, error) {

	actualFuncs := make(map[string]any)

	for _, curFuncs := range funcs {
		for k, v := range curFuncs {
			if _, exists := actualFuncs[k]; !exists {
				actualFuncs[k] = v
			}
		}
	}

	for k, v := range templatesFuncMap {
		if _, exists := actualFuncs[k]; !exists {
			actualFuncs[k] = v
		}
	}

	for k, v := range sprig.FuncMap() {
		if _, exists := actualFuncs[k]; !exists {
			actualFuncs[k] = v
		}
	}

	tmp, err := txtTemplate.New(name).Funcs(actualFuncs).Parse(templateTxt)
	if err != nil {
		return "", err
	}

	buf := strings.Builder{}
	err = tmp.Execute(&buf, params)

	if err != nil {
		return "", err
	}

	return buf.String(), nil
}
