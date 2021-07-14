#pragma once

namespace Examples
{
	class IExample
	{
	public:
		virtual void Render() = 0;

		virtual void BeforeResize() = 0;
		virtual void AfterResize() = 0;
		
		virtual ~IExample()
		{

		}
	};
}