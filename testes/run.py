import os, sys

def test_compiler(result, error):
    if result == 0:
        folder = "ok"
    else:
        folder = "erro"
    # para todos os arquivos da pasta, aplica os testes
    for file in os.listdir("../testes/{}".format(folder)):
        test = os.system("./" + sys.argv[1] + " < ../testes/{}/{} > /dev/null ".format(folder, file))
        if test != result:
            error = True
            print("Erro no teste: {}/{}".format(folder, file))
            print("---------------------------------------")
    
    return error

if __name__ == "__main__":
    if len(sys.argv) <= 1:
        print("Erro.\nUso: python run.py [folder]")
        print("Exemplo: python3 run.py etapa2")
    else:
        try:
            os.chdir("../" + sys.argv[1])
        except:
            print("Erro ao abrir esta pasta.")
        else:
            error = False
            os.system("make")
            print("---------------------------------------")
            error = test_compiler(0, error)             # Testes OK
            error = test_compiler(256, error)            # Testes com Erro
            if not error:
                print("Tudo OK!")
                print("---------------------------------------")
            os.system("make clean")