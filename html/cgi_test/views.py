#!/usr/bin/env python3

def read_file() -> int:
	try:
		with open("views_count", 'r') as file:
			content = file.read()
			if (content == ''):
				content = "0"
	except Exception as e:
		content = "0"
	return int(content)

def write_file(content):
	to_write = str(content)
	try:
		with open("views_count", 'w') as file:
			file.write(to_write)
	except FileNotFoundError:
		print(f"Error: File views_count not found.")
	except Exception as e:
		print(f"Error: {e}")

if __name__ == "__main__":
	count = read_file()
	count = count + 1
	print (f"<p>{count}<p>")
	write_file(count)

