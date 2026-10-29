// Trim façon radiocommande de modélisme (levier strié, rainure graduée,
// boutons de pas). Balisage attendu : voir .trim dans app.css et les pages
// Propulsion et Tourelle. onInput reçoit la nouvelle valeur entière ;
// l'appelant la répercute ensuite avec trim.set(valeur).

function createTrim(element, min, max, majorEvery, onInput)
{
	var vertical = element.classList.contains("trim-vertical");
	var track = element.querySelector(".trim-track");
	var rail = element.querySelector(".trim-rail");
	var fill = element.querySelector(".trim-fill");
	var lever = element.querySelector(".trim-lever");
	var trackPointer = null;
	var trim = { value: 0 };

	for(var tick = min; tick <= max; tick += majorEvery / 5)
	{
		var mark = document.createElement("span");
		mark.className = "trim-tick" + (tick === 0 ? " center" : (tick % majorEvery === 0 ? " major" : ""));
		mark.style[vertical ? "top" : "left"] = (vertical ? (max - tick) : (tick - min)) / (max - min) * 100 + "%";
		rail.appendChild(mark);
	}

	function percent(value)
	{
		return (value - min) / (max - min) * 100;
	}

	trim.set = function(value)
	{
		trim.value = value;
		var position = percent(value);
		var zero = percent(0);
		track.setAttribute("aria-valuemin", min);
		track.setAttribute("aria-valuemax", max);
		track.setAttribute("aria-valuenow", value);
		if(vertical)
		{
			lever.style.top = (100 - position) + "%";
			fill.style.top = (100 - Math.max(position, zero)) + "%";
			fill.style.height = Math.abs(position - zero) + "%";
		}
		else
		{
			lever.style.left = position + "%";
			fill.style.left = Math.min(position, zero) + "%";
			fill.style.width = Math.abs(position - zero) + "%";
		}
	};

	function emit(value)
	{
		onInput(Math.max(min, Math.min(max, Math.round(value))));
	}

	function valueFromPointer(event)
	{
		var bounds = rail.getBoundingClientRect();
		var ratio = vertical
			? 1 - (event.clientY - bounds.top) / bounds.height
			: (event.clientX - bounds.left) / bounds.width;
		return min + Math.max(0, Math.min(1, ratio)) * (max - min);
	}

	track.addEventListener("pointerdown", function(event)
	{
		trackPointer = event.pointerId;
		track.setPointerCapture(event.pointerId);
		track.focus({ preventScroll: true });
		emit(valueFromPointer(event));
		event.preventDefault();
	});
	track.addEventListener("pointermove", function(event)
	{
		if(event.pointerId === trackPointer)
		{
			emit(valueFromPointer(event));
		}
	});
	track.addEventListener("pointerup", function() { trackPointer = null; });
	track.addEventListener("pointercancel", function() { trackPointer = null; });
	track.addEventListener("keydown", function(event)
	{
		var steps = { ArrowUp: 1, ArrowRight: 1, ArrowDown: -1, ArrowLeft: -1, PageUp: 10, PageDown: -10 };
		if(steps[event.key])
		{
			emit(trim.value + steps[event.key]);
			event.preventDefault();
		}
		else if(event.key === "Home" || event.key === "End")
		{
			emit(event.key === "Home" ? min : max);
			event.preventDefault();
		}
	});

	var buttons = element.querySelectorAll(".trim-step");
	for(var i = 0; i < buttons.length; i++)
	{
		buttons[i].addEventListener("click", function()
		{
			emit(trim.value + Number(this.getAttribute("data-step")));
		});
	}

	trim.set(0);
	return trim;
}
