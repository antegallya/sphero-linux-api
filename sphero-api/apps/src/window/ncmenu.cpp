
#include <sstream>
#include <string>
#include <ncurses.h>
#include <menu.h>

#include <iostream>
using namespace std;


#include "ncmenu.h"
#include "keys.hpp"


Menu::Menu(string name):
	_menuName(name), _my_items(NULL), _my_menu(NULL), _nbItems(0), _curItem(0)
{}

Menu::~Menu()
{
	unpost_menu(_my_menu);
	free_menu(_my_menu);

	for(int i = 0; i < _nbItems; ++i)
		free_item(_my_items[i]);
	free(_my_items);


	for(MenuItem *mi : _itemList)
		delete mi;
}

void Menu::addItem(MenuItem *m)
{
	_itemList.push_back(m);
}

void Menu::removeItem(int index)
{
	if(index >= _itemList.size())
		return;

	_itemList.erase(_itemList.begin() + index);
}

void Menu::initMenu()
{
	_curItem = 0;

	if(_my_items != NULL)
	{
		for(int i = 0; i < _nbItems; ++i)
			free_item(_my_items[i]);
		free(_my_items);
	}

	if(_my_menu != NULL)
	{
		free_menu(_my_menu);
	}

	_nbItems = _itemList.size();

	_my_items = (ITEM **)calloc(_nbItems + 1, sizeof(ITEM *));

	for(int i = 0; i < _nbItems; ++i)
	{
		_my_items[i] = new_item(_itemList[i]->itemText().c_str(), "");
	}
	_my_items[_nbItems] = (ITEM *)NULL;

	_my_menu = new_menu((ITEM **)_my_items);
	menu_opts_off(_my_menu, O_SHOWDESC);
	set_menu_mark(_my_menu, "*");
}


void Menu::showMenu(int ch, WINDOW *w)
{
	set_menu_win(_my_menu, w);
	switch(ch)
	{
		case KEY_DOWN:
			if(menu_driver(_my_menu, REQ_DOWN_ITEM) == E_OK)
			{
				_curItem++;
			}
			break;
		case KEY_UP:
			if(menu_driver(_my_menu, REQ_UP_ITEM) == E_OK)
			{
				_curItem--;
			}
			break;
		case KEY_RETURN:
			_itemList[_curItem]->doAction();
			break;
	}
	post_menu(_my_menu);
}

void Menu::hideMenu(int ch, WINDOW *w)
{
	unpost_menu(_my_menu);
}

string Menu::getName()
{
	return _menuName;
}

void Menu::getSize(int &height, int &width)
{
	height = _itemList.size();

	width = 0;
	for(MenuItem* mi : _itemList)
		if(width < mi->itemText().size())
			width = mi->itemText().size();

}

