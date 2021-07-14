#pragma once

namespace Examples
{
	class IExample
	{
	public:
		virtual void Render() = 0;
		
		virtual ~IExample()
		{

		}
	};
}