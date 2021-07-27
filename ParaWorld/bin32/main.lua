--[[
Title: Main loop
Author(s): LiXizhi
Company: ParaEnging Co.
Date: 2014/3/21
Desc: Entry point and game loop
use the lib:
------------------------------------------------------------
NPL.activate("source/HelloWorld/main.lua");
Or run application with command line: bootstrapper="source/HelloWorld/main.lua"
------------------------------------------------------------
]]
-- ParaWorld platform includes
-- NPL.load("(gl)script/kids/ParaWorldCore.lua"); 

local main_state = nil;

function SceneTest()
	local asset = ParaAsset.LoadStaticMesh("","model/boy01.x");
	local obj = ParaScene.CreateMeshPhysicsObject("blueprint_center", asset, 1,1,1, false, "1,0,0,0,1,0,0,0,1,0,0,0");
	obj:SetPosition(ParaScene.GetPlayer():GetPosition());
	obj:SetField("progress",1);
	obj:GetEffectParamBlock():SetBoolean("ztest", false);
	obj:SetField("RenderOrder", 101)
	ParaScene.Attach(obj);
end

local function InitApp()
	local _this = ParaUI.CreateUIObject("button", "my_button", "_ct", -50, 0, 100, 50);
	_this.text = "HelloWorld CF!";
	_this.background = "";
	_this:AttachToRoot();

	SceneTest();
end

local function activate()
	if(main_state == nil) then
		InitApp();
		main_state = 1;
	else
		log("hello world from main loop\n");
	end
end

NPL.this(activate);