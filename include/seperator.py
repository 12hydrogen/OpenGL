import re

with open("./interface.hpp", "r") as header:
    with open("./interface.cpp", "w") as body:
        body.write('#include "interface.hpp"\n')
        content = header.readlines()
        namespace = ''
        currentClass = ''
        currentFunc = ''
        currentFuncPrefix = ''
        funcLines: dict[str, list] = dict()
        quoteCount = 0
        for line in range(0, len(content)):
            matchs = re.match(r'^\t*namespace ([a-zA-Z]+){1};?', content[line])
            if matchs is not None:
                if matchs.group().endswith(';'):
                    continue
                namespace = matchs.group(1).strip()
                continue
            matchs = re.match(r'^\t*class ([a-zA-Z]+){1};?', content[line])
            if matchs is not None:
                if matchs.group().endswith(';'):
                    continue
                currentClass = matchs.group(1).strip()
                continue
            matchs = re.match('^\t*(?:[a-zA-Z]+ +)*void +([a-zA-Z]+)(\([a-zA-Z=\*,\s]+\))*', content[line])
            if matchs is not None:
                if matchs.group().endswith(';'):
                    continue
                currentFunc = matchs.group(1).strip()
                print(matchs.group().strip())
                print(matchs.group().strip().replace(currentFunc, f'{currentClass}::{currentFunc}'))
                funcLines[f'{currentClass}::{currentFunc}'] = [quoteCount + 1, matchs.group().strip().replace(currentFunc, f'{currentClass}::{currentFunc}', 1), line]
                continue
            matchs = re.match(r'^.*({.*)', content[line])
            if matchs is not None:
                quoteCount += 1
                #print(f'Start of class and func. {quoteCount} {namespace}::{currentClass}::{currentFunc}')
            matchs = re.match(r'^(.*}).*', content[line])
            if matchs is not None:
                for name in funcLines.keys():
                    if currentFunc in name:
                        if funcLines[name][0] == quoteCount:
                            funcLines[name].append(line)
                #print(f'  End of class and func. {quoteCount} {namespace}::{currentClass}::{currentFunc}')
                quoteCount -= 1
        # print(funcLines)
        for name in funcLines.keys():
            body.write(funcLines[name][1] + '\n')
            for i in range(funcLines[name][2] + 1, funcLines[name][-1] + 1):
                body.write(content[i][2:])
