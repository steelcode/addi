from itertools import islice
import networkx as nx

tag = "ANDROID_NODE"


def check_item(item):
	if "Landroid/" in item or "Lcom/android" in item:
		return True
	else:
		return False

def prova(l):
	global g, tag
	chiamante = None
	#top stacktrace
	print 'aggiungo solo ' + l[0]
	chiamante = l[0]
	if check_item(chiamante):
		g.add_node(tag)
	else:
		g.add_node(chiamante)
	_ = l.pop(0)
	for item in l:
		print 'aggiungo nodo' + chiamante + ' chiamato da: ' + item
		if check_item(chiamante):
			if check_item(item):
				#android call android
				continue
			else:
				g.add_edge(item,tag)
		else:
			if check_item(item):
				g.add_edge(tag,chiamante)
			else:
				g.add_edge(item,chiamante)

		#g.add_edge(item,chiamante)
		chiamante = item
		#add(item,n)


def strace_reader():
	global  g
	g = nx.DiGraph()
	#prima riga e il numero di chiamate = n
	#le successive n righe sono le chiamate, poi si ripete
	with open("/tmp/stacktraces.log") as myfile:
		while True:
			#leggo la len
			try:
				num = next(myfile)
			except:
				print 'fine'
				break
			print num
			#salvo le successive l righe in un array di l elementi
			lista = list(islice(myfile, int(num)))
			#aggiungo i nodi
			print lista
			prova(lista)

strace_reader()
print(g.nodes())
nx.write_gexf(g,"trace.gexf")
