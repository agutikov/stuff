

$fn=36;


module arc (r_inner, r_outer, l) {
		intersection() {
			rotate([0, 90, 0])
			difference(){
				cylinder(d=2*r_outer, h=l);
				cylinder(d=2*r_inner, h=l);
			}
			translate([0, -r_outer, -r_outer])
			cube([l, r_outer, r_outer]);
		}
}

/*****************
	Уголок профильный:
	s - толщина полки
	h - одна полка
	w - вторая полка
	l - длинна
	r - радиус закругления
******************/

module profile_L (s, h, w, l, r) {
	union() {
		translate([0, r, r])
		arc(r-s, r, l);
		translate([0, 0, r])
		cube([l, s, h-r]);
		translate([0, r, 0])
		cube([l, w-r, s]);
	}
}

//profile_L(5, 20, 50, 300, 6);


/***********************
	Труба профильная прямоугольная:
	s - толщина стенки
	h - толщина трубы 
	w - толщина трубы вторая
	l - длинна
	r - радиус закругления
************************/

module profile_pipe (s, h, w, l, r) {
	union () {
		translate([0, r, r])
		arc(r-s, r, l);

		translate([0, w-r, r])
		rotate([90, 0, 0])
		arc(r-s, r, l);

		translate([0, w-r, h-r])
		rotate([180, 0, 0])
		arc(r-s, r, l);

		translate([0, r, h-r])
		rotate([270, 0, 0])
		arc(r-s, r, l);

		translate([0, 0, r])
		cube([l, s, h-2*r]);

		translate([0, w-s, r])
		cube([l, s, h-2*r]);

		translate([0, r, 0])
		cube([l, w-2*r, s]);

		translate([0, r, h-s])
		cube([l, w-2*r, s]);
	}
}

//profile_pipe(3, 50, 30, 300, 4);


/*************************
	Используемые профили:
	- труба квадратная 50x50x3
	- уголок равнополочный 50x50x5
**************************/

pipe_w = 50;
pipe_s = 5;
pipe_r = 6;
L_w = 50;
L_s = 3;
L_r = 4;

module L (l) {
	profile_L(L_s, L_w, L_w, l, L_r);
}

module pipe (l) {
	profile_pipe(pipe_s, pipe_w, pipe_w, l, pipe_r);
}

/********************
	Размеры
*********************/

work_area_x = 200;
work_area_y = 200;
operation_area_x = 2*work_area_x;
operation_area_y = 2*work_area_y;
mount_width = 150;
vertical_height = 150;
mill_mount_w = 70;
arrow_len = work_area_x - mill_mount_w;


/********************
	Элементы конструкции:
	- base - основание
	- column - колонна
	- arrow - вылет для крепления
*********************/

module I (l, w1, w2) {
	pipe(l);

	translate([-L_r, -(w1/2 - pipe_w/2), pipe_w+L_s])
	rotate([0, 180, -90])
	L(w1);

	translate([l + L_r, w2/2 + pipe_w/2, pipe_w+L_s])
	rotate([0, 180, 90])
	L(w2);
}


module II (l, w1, w2, dw) {
	translate([0, dw/2 + pipe_w/2, 0])
	pipe(l);

	translate([0, -(dw/2 + pipe_w/2), 0])
	pipe(l);

	translate([-L_r, -(w1/2 - pipe_w/2), pipe_w+L_s])
	rotate([0, 180, -90])
	L(w1);

	translate([l + L_r, w2/2 + pipe_w/2, pipe_w+L_s])
	rotate([0, 180, 90])
	L(w2);
}

// полная длинна основания: 400 + 150 + 50 = 600
base_length = operation_area_x + mount_width + L_w;

module base() {
	pipe(base_length);

	translate([base_length + L_r, (operation_area_y + pipe_w)/2, pipe_w+L_s])
	rotate([0, 180, 90])
	L(operation_area_y);
}

col_h = vertical_height + (mount_width-pipe_w)/2;

module column() {
	translate([0, 0, L_r])
	rotate([0, -90, 90])
//	II(col_h, mount_width, mount_width, mount_width - 2*pipe_w - 2*35);
	I(col_h, mount_width, mount_width);
}

// полная высота колонны с креплениями
column_height = col_h + 2*L_r;


module arrow () {
	pipe(arrow_len + mount_width);

	translate([0, 0, (mount_width + pipe_w)/2])
	rotate([0, 90, 270])
	L(mount_width);

	translate([0, pipe_w, (mount_width + pipe_w)/2])
	rotate([0, 90, 0])
	L(mount_width);
}

/******************
	Сборка с отверстиями
*******************/

difference() {
	union () {
		translate([arrow_len + mount_width, pipe_w, 0])
		rotate([0, 0, 180])
		base();

		translate([arrow_len + (mount_width + pipe_w)/2, pipe_w, pipe_w + 50])
		column();

		translate([0, 0, column_height + pipe_w + 100])
		arrow();
	}
	union() {
		translate([arrow_len + 20, L_w/2, -column_height])
		cylinder(d=8, h=7*column_height, center=true);

		translate([arrow_len + mount_width - 20, L_w/2, -column_height])
		cylinder(d=8, h=7*column_height, center=true);




	}
}

























