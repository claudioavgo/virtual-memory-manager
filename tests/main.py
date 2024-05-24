import os
import random

def generate_test_files(num_files, num_numbers, min_repeat_rate, max_repeat_rate, output_dir="test/addresses"):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    for i in range(num_files):
        # Determinando a taxa de repetição para o arquivo atual
        repeat_rate = random.uniform(min_repeat_rate, max_repeat_rate)
        unique_count = int(num_numbers * (1 - repeat_rate))
        repeat_count = num_numbers - unique_count
        
        # Gerando números únicos
        unique_numbers = random.sample(range(65536), unique_count)
        
        # Gerando números repetidos
        repeat_numbers = random.choices(unique_numbers, k=repeat_count)
        
        # Combinando os números únicos e repetidos
        numbers = unique_numbers + repeat_numbers
        
        # Embaralhando os números para misturar únicos e repetidos
        random.shuffle(numbers)
        
        # Escrevendo os números no arquivo
        file_path = os.path.join(output_dir, f"addresses_{i+1}.txt")
        with open(file_path, 'w') as f:
            for number in numbers:
                f.write(f"{number}\n")

# Variáveis de configuração
num_files = 5           # Quantos arquivos criar
num_numbers = 2500      # Quantos números em cada arquivo
min_repeat_rate = 0.4   # Taxa mínima de repetição (10%)
max_repeat_rate = 0.6   # Taxa máxima de repetição (50%)

# Gerando os arquivos de teste
generate_test_files(num_files, num_numbers, min_repeat_rate, max_repeat_rate)

print(f"{num_files} arquivos de teste foram gerados na pasta 'test_files'.")