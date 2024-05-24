#!/bin/bash

# Função para executar testes
run_tests() {
  command=$1
  echo "---------------------------------"
  echo "Executando testes para $command"
  echo "---------------------------------"

  # Loop through the input files
  for inputfile in ./tests/addresses/*.txt
  do
    # Extrair o número do arquivo de entrada
    filenumber=$(basename "$inputfile" .txt | sed 's/addresses_//')

    comparefile="./tests/corrects/$command/correct_$filenumber.txt"

    # Executar o comando no arquivo de entrada
    ./vm $inputfile $command

    # Comparar a saída com o arquivo comparefile
    total_lines=$(wc -l < "$comparefile")
    diff_lines=$(diff -y --suppress-common-lines correct.txt "$comparefile" | wc -l)
    same_lines=$((total_lines - diff_lines))
    
    # Calcular a porcentagem de similaridade
    if [ $total_lines -gt 0 ]
    then
      similarity=$((100 * same_lines / total_lines))
    else
      similarity=0
    fi

    if diff -a correct.txt "$comparefile" &> /dev/null
    then
      echo "Teste $filenumber: Passou"
      echo "---------------------------------"
    else
      echo "Teste $filenumber: Não passou"
      echo
      # print the diff
      diff -a correct.txt "$comparefile"
      echo
      echo "Similaridade: $similarity%"
      echo "---------------------------------"
    fi
  done
}

# Executar testes para rate e edf sequencialmente
run_tests fifo
run_tests lru