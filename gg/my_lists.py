#!/usr/bin/pyhton



import sys
from pprint import pprint
import types





'''
	деревья - списки списков
	узел дерева - список
	лист - кусок данных
	кусок данных отдельно - сразу единственный лист в дереве (так чтобы любые куски данных сразу были деревьями)
	полноценный список - это dict где каждое значение - список, а ключи могут быть числами, строками или кортежами

	вопрос - как быть с типами данных - числами, которые можно преобразовывать в строки какого-то формата и наоборот,
	и байтовыми массивами которые можно преобразовывать в строки или числа и наоборот

	ключи и значения можно поменять местами если значения одноуровневые списки - это одна из базовых операций
	кортежи при этом сохраняются и не преобразовываются в списки и обратно
	список пар преобразуется в dict и обратно
	получить список ключей и список значений отдельно
	преобразовать список в dict где элементы списка будут ключами а значения отсутсвуют (заменены плейсхолдерами)
	слить два списка одинаковой длинны в dict
	слить два списка возможно разной длины в dict с использованием словаря индекс-индекс
	подставить значения из списка в словарь индекс-значение или ключ-индекс
	замаппить dict ключ1-ключ2 и dict ключ2-значение в dict ключ1-значение

	Уровни организации:
	1) типы значений и преобразования
	2) внутренняя структура деревьев, преобразования массивов, словарей и т.д.
	3) рекурсивные оперции с деревьями
	4) рекурсивные оперции с деревьями с использванием плейсхолдеров

'''














